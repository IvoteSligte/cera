package main

import (
	"context"
	"log"
	"unsafe"

	"github.com/owenrumney/go-lsp/document"
	"github.com/owenrumney/go-lsp/lsp"
	"github.com/owenrumney/go-lsp/server"
)

//go:generate make -C .. build/lib.o BUILD=lib
/*
#include "compiler.h"
*/
import "C"

type handler struct {
	docs   *document.Store
	client *server.Client
}

func (h *handler) Initialize(_ context.Context, _ *lsp.InitializeParams) (*lsp.InitializeResult, error) {
	return &lsp.InitializeResult{
		ServerInfo: &lsp.ServerInfo{Name: "ceam-ls", Version: "0.1.0"},
	}, nil
}

func (h *handler) Shutdown(_ context.Context) error { return nil }

func (h *handler) SetClient(client *server.Client) {
	h.client = client
}

func (h *handler) DidOpen(_ context.Context, params *lsp.DidOpenTextDocumentParams) error {
	_, err := h.docs.Open(params)
	return err
}

func (h *handler) DidChange(_ context.Context, params *lsp.DidChangeTextDocumentParams) error {
	_, err := h.docs.Change(params)
	return err
}

func (h *handler) DidClose(_ context.Context, params *lsp.DidCloseTextDocumentParams) error {
	h.docs.Close(params)
	return nil
}

func (h *handler) DidSave(ctx context.Context, params *lsp.DidSaveTextDocumentParams) error {
	_ = h.client.LogMessage(ctx, &lsp.LogMessageParams{
		Type:    lsp.MessageTypeInfo,
		Message: "document saved: " + string(params.TextDocument.URI),
	})
	text, ok := h.docs.Text(params.TextDocument.URI)
	var diags []lsp.Diagnostic
	if ok {
		ctext := C.CString(text)
		cast := C.AST{}
		cerrors := C.CompileErrors{}
		if !C.compile(ctext, &cast, &cerrors) {
			var errors []C.CompileError = unsafe.Slice(cerrors.data, cerrors.length)
			for _, error := range errors {
				sev := lsp.SeverityError
				diags = append(diags, lsp.Diagnostic{
					Range: lsp.Range{
						Start: lsp.Position{Line: int(error.line), Character: int(error.column)},
						End:   lsp.Position{Line: int(error.line), Character: int(error.column) + 1},
					},
					Severity: &sev,
					Source:   "compiler",
					Message:  "TODO found",
				})
			}
		}
		C.free(unsafe.Pointer(ctext))
	}
	return h.client.PublishDiagnostics(ctx, &lsp.PublishDiagnosticsParams{
		URI:         params.TextDocument.URI,
		Diagnostics: diags,
	})
}

func main() {
	h := &handler{docs: document.NewStore()}
	srv := server.NewServer(h)
	if err := srv.Run(context.Background(), server.RunStdio()); err != nil {
		log.Fatal(err)
	}
}
