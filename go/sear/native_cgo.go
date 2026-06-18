//go:build sear_native

package sear

/*
#cgo CFLAGS: -I../../sear
#include <stdbool.h>
#include <stdlib.h>
#include "sear.h"
*/
import "C"

import (
	"sync"
	"unsafe"
)

var searMutex sync.Mutex

func callSearNative(requestJSON []byte, debug bool) (*NativeResponse, error) {
	if len(requestJSON) == 0 {
		return nil, &NativeError{Message: "request_json must not be empty"}
	}

	requestCString := C.CString(string(requestJSON))
	defer C.free(unsafe.Pointer(requestCString))

	searMutex.Lock()
	result := C.sear(requestCString, C.int(len(requestJSON)), C.bool(debug))
	searMutex.Unlock()

	if result == nil {
		return nil, &NativeError{Message: "sear() returned nil result"}
	}

	response := &NativeResponse{}

	if result.raw_request != nil && result.raw_request_length > 0 {
		response.RawRequest = C.GoBytes(unsafe.Pointer(result.raw_request), C.int(result.raw_request_length))
	}

	if result.raw_result != nil && result.raw_result_length > 0 {
		response.RawResult = C.GoBytes(unsafe.Pointer(result.raw_result), C.int(result.raw_result_length))
	}

	if result.result_json != nil && result.result_json_length > 0 {
		response.ResultJSON = C.GoBytes(unsafe.Pointer(result.result_json), C.int(result.result_json_length))
	}

	return response, nil
}
