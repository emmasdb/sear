package sear

import (
	"errors"
	"testing"
)

func TestSearReturnsSecurityResult(t *testing.T) {
	originalBridge := callSearBridge
	defer func() {
		callSearBridge = originalBridge
	}()

	callSearBridge = func(_ []byte, _ bool) (*NativeResponse, error) {
		return &NativeResponse{
			RawRequest: []byte("req"),
			RawResult:  []byte("res"),
			ResultJSON: []byte(`{"status":"ok"}`),
		}, nil
	}

	request := map[string]any{
		"operation":  "extract",
		"admin_type": "user",
		"userid":     "MYUSER",
	}

	result, err := Sear(request, false)
	if err != nil {
		t.Fatalf("expected no error, got %v", err)
	}

	if result.Result["status"] != "ok" {
		t.Fatalf("expected result.status == ok, got %#v", result.Result["status"])
	}

	if string(result.RawRequest) != "req" {
		t.Fatalf("expected raw request to be copied")
	}

	if string(result.RawResult) != "res" {
		t.Fatalf("expected raw result to be copied")
	}
}

func TestSearJSONReturnsJSONError(t *testing.T) {
	originalBridge := callSearBridge
	defer func() {
		callSearBridge = originalBridge
	}()

	callSearBridge = func(_ []byte, _ bool) (*NativeResponse, error) {
		return &NativeResponse{ResultJSON: []byte("not-json")}, nil
	}

	_, err := SearJSON([]byte(`{"operation":"extract"}`), false)
	if err == nil {
		t.Fatalf("expected JSONError")
	}

	var jsonErr *JSONError
	if !errors.As(err, &jsonErr) {
		t.Fatalf("expected JSONError, got %T", err)
	}
}

func TestSearJSONReturnsRequestError(t *testing.T) {
	originalBridge := callSearBridge
	defer func() {
		callSearBridge = originalBridge
	}()

	callSearBridge = func(_ []byte, _ bool) (*NativeResponse, error) {
		return &NativeResponse{ResultJSON: []byte(`{"ok":true}`)}, nil
	}

	_, err := SearJSON([]byte("not-json"), false)
	if err == nil {
		t.Fatalf("expected RequestError")
	}

	var requestErr *RequestError
	if !errors.As(err, &requestErr) {
		t.Fatalf("expected RequestError, got %T", err)
	}
}

func TestCallSearPropagatesNativeError(t *testing.T) {
	originalBridge := callSearBridge
	defer func() {
		callSearBridge = originalBridge
	}()

	callSearBridge = func(_ []byte, _ bool) (*NativeResponse, error) {
		return nil, &NativeError{Message: "bridge failed"}
	}

	_, err := CallSear([]byte(`{"operation":"extract"}`), false)
	if err == nil {
		t.Fatalf("expected NativeError")
	}

	var nativeErr *NativeError
	if !errors.As(err, &nativeErr) {
		t.Fatalf("expected NativeError, got %T", err)
	}
}
