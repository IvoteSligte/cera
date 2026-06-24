use std::collections::HashMap;
use std::error::Error;
use std::ffi::{CStr, CString};
use std::str::FromStr;
use std::time::Instant;

use anyhow::Result;
use lsp_server::{
    Connection, Message, Notification, Request as ServerRequest, RequestId, Response,
};
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
            "change": 2, // incremental updates
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
    conn: Connection,
    params: serde_json::Value,
) -> std::result::Result<(), Box<dyn Error + Sync + Send>> {
    let _init: InitializeParams = serde_json::from_value(params)?;
    let mut documents = HashMap::new();
    let mut prev_diagnostics = Vec::new();

    for msg in &conn.receiver {
        eprintln!("MSG: {:#?}", msg);
        match msg {
            Message::Request(req) => {
                if conn.handle_shutdown(&req)? {
                    break;
                }
                if let Err(err) = handle_request(&conn, &req) {
                    log::error!("[lsp] request {} failed: {err}", &req.method);
                }
            }
            Message::Notification(notif) => {
                if let Err(err) =
                    handle_notification(&conn, &notif, &mut documents, &mut prev_diagnostics)
                {
                    log::error!("[lsp] notification {} failed: {err}", notif.method);
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
fn position_to_offset(pos: Position, text: &str) -> Option<usize> {
    let mut offset = 0;
    for (i, line) in text.lines().enumerate() {
        if i == pos.line as usize {
            if pos.character as usize > line.len() {
                return Some(offset + line.len().saturating_sub(1));
            }
            return Some(offset + pos.character as usize);
        }
        offset += line.len() + 1;
    }
    return None;
}

fn to_offset_range(
    Range { start, end }: lsp_types::Range,
    text: &str,
) -> Option<std::ops::Range<usize>> {
    Some(
        position_to_offset(start, text).unwrap_or(text.len())
            ..position_to_offset(end, text).unwrap_or(text.len()),
    )
}

fn handle_notification(
    conn: &Connection,
    notif: &Notification,
    documents: &mut HashMap<Uri, String>,
    prev_diagnostics: &mut Vec<Diagnostic>,
) -> Result<()> {
    match notif.method.as_str() {
        DidOpenTextDocument::METHOD => {
            let p: DidOpenTextDocumentParams = serde_json::from_value(notif.params.clone())?;
            let uri = p.text_document.uri;
            let text = documents
                .entry(uri.clone())
                .insert_entry(p.text_document.text);
            publish_diagnostics(conn, uri, text.get(), prev_diagnostics)?;
        }
        DidChangeTextDocument::METHOD => {
            let p: DidChangeTextDocumentParams = serde_json::from_value(notif.params.clone())?;
            if let Some(change) = p.content_changes.into_iter().next() {
                let uri = p.text_document.uri;
                let Some(range) = to_offset_range(change.range.unwrap(), &documents[&uri]) else {
                    panic!(
                        "Invalid range: {:?} in text {}",
                        change.range.unwrap(),
                        &documents[&uri]
                    );
                };
                eprintln!("range: {}..{}", range.start, range.end);
                let text = documents.get_mut(&uri).unwrap();
                text.replace_range(range, &change.text);
                eprintln!("text: `{}`", text);
                publish_diagnostics(conn, uri, text, prev_diagnostics)?;
            }
        }
        DidSaveTextDocument::METHOD => {
            let p: DidSaveTextDocumentParams = serde_json::from_value(notif.params.clone())?;
            if let Some(text) = p.text {
                let uri = p.text_document.uri;
                publish_diagnostics(conn, uri, &text, prev_diagnostics)?;
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
macro_rules! time {
    ($expr: expr) => {{
        let start = Instant::now();
        let output = $expr;
        let end = Instant::now();
        (output, end - start)
    }};
}

fn get_diagnostics(text: &str) -> Result<Vec<Diagnostic>> {
    unsafe {
        let (raw_errors, time) = time!(ceam::diagnose(CString::from_str(text)?.as_ptr()));
        eprintln!("Time to diagnose: {:?}", time);

        if raw_errors.data.is_null() {
            // cannot create slice from null pointer
            return Ok(Vec::new());
        }
        std::slice::from_raw_parts(raw_errors.data, raw_errors.length)
            .iter()
            .map(|error| {
                let message = CStr::from_ptr(error.message).to_str()?.to_owned();
                let line = error.line as u32 - 1; // ceam line numbers are one-indexed
                let column = error.column as u32;
                let length = error.length as u32;
                eprintln!("Line: {}, column: {}", line, column);
                Ok(Diagnostic {
                    range: Range::new(
                        Position::new(line, column),
                        Position::new(line, column + length),
                    ),
                    severity: Some(DiagnosticSeverity::ERROR),
                    code: None,
                    code_description: None,
                    source: Some("ceam_lsp".into()),
                    message: message,
                    related_information: None,
                    tags: None,
                    data: None,
                })
            })
            .collect()
    }
}

fn publish_diagnostics(
    conn: &Connection,
    uri: Uri,
    text: &str,
    prev_diagnostics: &mut Vec<Diagnostic>,
) -> Result<()> {
    let diagnostics = get_diagnostics(text)?;
    if diagnostics == *prev_diagnostics {
        return Ok(());
    }
    *prev_diagnostics = diagnostics.clone();
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
