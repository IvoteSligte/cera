use std::error::Error;
use std::ffi::{CStr, CString};
use std::str::FromStr;
use std::time::Instant;

use anyhow::Result;
use lsp_server::{Connection, Message, Request as ServerRequest, RequestId, Response};
use lsp_types::DidSaveTextDocumentParams;
use lsp_types::notification::{DidSaveTextDocument, Notification as _}; // for METHOD consts
use lsp_types::request::Request as _;
use lsp_types::{
    CompletionItem,
    CompletionItemKind,
    // capability helpers
    CompletionResponse,
    Diagnostic,
    DiagnosticSeverity,
    DidChangeTextDocumentParams,
    DidOpenTextDocumentParams,
    Hover,
    HoverContents,
    // core
    InitializeParams,
    MarkedString,
    Position,
    PublishDiagnosticsParams,
    Range,
    Uri,
    // notifications
    notification::{DidChangeTextDocument, DidOpenTextDocument, PublishDiagnostics},
    // requests
    request::{Completion, GotoDefinition, HoverRequest},
}; // for METHOD consts

mod ceam;

// =====================================================================
// main
// =====================================================================

#[allow(clippy::print_stderr)]
fn main() -> std::result::Result<(), Box<dyn Error + Sync + Send>> {
    log::error!("starting Ceam LSP");

    // transport
    let (connection, io_thread) = Connection::stdio();

    // advertised capabilities
    let init_value = serde_json::json!({
        "textDocumentSync": {
            "change": 1,
            "openClose": true
        }
    });
    let init_params = connection.initialize(init_value)?;
    main_loop(connection, init_params)?;
    io_thread.join()?;
    log::error!("shutting down server");
    Ok(())
}

// =====================================================================
// event loop
// =====================================================================

fn main_loop(
    connection: Connection,
    params: serde_json::Value,
) -> std::result::Result<(), Box<dyn Error + Sync + Send>> {
    let _init: InitializeParams = serde_json::from_value(params)?;

    for msg in &connection.receiver {
        eprintln!("MSG: {:?}", msg);
        match msg {
            Message::Request(req) => {
                if connection.handle_shutdown(&req)? {
                    break;
                }
                if let Err(err) = handle_request(&connection, &req) {
                    log::error!("[lsp] request {} failed: {err}", &req.method);
                }
            }
            Message::Notification(note) => {
                if let Err(err) = handle_notification(&connection, &note) {
                    log::error!("[lsp] notification {} failed: {err}", note.method);
                }
            }
            Message::Response(resp) => log::error!("[lsp] response: {resp:?}"),
        }
    }
    Ok(())
}

// =====================================================================
// notifications
// =====================================================================

fn handle_notification(conn: &Connection, note: &lsp_server::Notification) -> Result<()> {
    match note.method.as_str() {
        DidOpenTextDocument::METHOD => {
            let p: DidOpenTextDocumentParams = serde_json::from_value(note.params.clone())?;
            let uri = p.text_document.uri;
            publish_diagnostics(conn, uri, &p.text_document.text)?;
        }
        DidChangeTextDocument::METHOD => {
            let p: DidChangeTextDocumentParams = serde_json::from_value(note.params.clone())?;
            if let Some(change) = p.content_changes.into_iter().next() {
                let uri = p.text_document.uri;
                publish_diagnostics(conn, uri, &change.text)?;
            }
        }
        DidSaveTextDocument::METHOD => {
            let p: DidSaveTextDocumentParams = serde_json::from_value(note.params.clone())?;
            if let Some(text) = p.text {
                let uri = p.text_document.uri;
                publish_diagnostics(conn, uri, &text)?;
            }
        }
        _ => {}
    }
    Ok(())
}

// =====================================================================
// requests
// =====================================================================

fn handle_request(conn: &Connection, req: &ServerRequest) -> Result<()> {
    match req.method.as_str() {
        GotoDefinition::METHOD => {
            send_ok(
                conn,
                req.id.clone(),
                &lsp_types::GotoDefinitionResponse::Array(Vec::new()),
            )?;
        }
        Completion::METHOD => {
            let item = CompletionItem {
                label: "HelloFromLSP".into(),
                kind: Some(CompletionItemKind::FUNCTION),
                detail: Some("dummy completion".into()),
                ..Default::default()
            };
            send_ok(conn, req.id.clone(), &CompletionResponse::Array(vec![item]))?;
        }
        HoverRequest::METHOD => {
            let hover = Hover {
                contents: HoverContents::Scalar(MarkedString::String(
                    "Hello from *Ceam LSP*".into(),
                )),
                range: None,
            };
            send_ok(conn, req.id.clone(), &hover)?;
        }
        _ => send_err(
            conn,
            req.id.clone(),
            lsp_server::ErrorCode::MethodNotFound,
            "unhandled method",
        )?,
    }
    Ok(())
}

// =====================================================================
// diagnostics
// =====================================================================
fn publish_diagnostics(conn: &Connection, uri: Uri, text: &str) -> Result<()> {
    let mut diagnostics = Vec::new();
    unsafe {
        let start = Instant::now();
        let raw_errors = ceam::diagnose(CString::from_str(text)?.as_ptr());
        let end = Instant::now();
        eprintln!("Time to diagnose: {:?}", end - start);

        if raw_errors.data.is_null() {
            return Ok(());
        }
        for error in std::slice::from_raw_parts(raw_errors.data, raw_errors.length) {
            let message = CStr::from_ptr(error.message).to_str()?.to_owned();
            let line = error.line as u32;
            let column = error.column as u32;
            diagnostics.push(Diagnostic {
                range: Range::new(Position::new(line, column), Position::new(line, column + 1)),
                severity: Some(DiagnosticSeverity::ERROR),
                code: None,
                code_description: None,
                source: Some("ceam_lsp".into()),
                message: message,
                related_information: None,
                tags: None,
                data: None,
            });
        }
    }
    let params = PublishDiagnosticsParams {
        uri,
        diagnostics,
        version: None,
    };
    conn.sender
        .send(Message::Notification(lsp_server::Notification::new(
            PublishDiagnostics::METHOD.to_owned(),
            params,
        )))?;
    Ok(())
}

// =====================================================================
// helpers
// =====================================================================

fn send_ok<T: serde::Serialize>(conn: &Connection, id: RequestId, result: &T) -> Result<()> {
    let resp = Response {
        id,
        result: Some(serde_json::to_value(result)?),
        error: None,
    };
    conn.sender.send(Message::Response(resp))?;
    Ok(())
}

fn send_err(
    conn: &Connection,
    id: RequestId,
    code: lsp_server::ErrorCode,
    msg: &str,
) -> Result<()> {
    let resp = Response {
        id,
        result: None,
        error: Some(lsp_server::ResponseError {
            code: code as i32,
            message: msg.into(),
            data: None,
        }),
    };
    conn.sender.send(Message::Response(resp))?;
    Ok(())
}
