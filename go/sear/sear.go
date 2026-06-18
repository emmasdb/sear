package sear

import "encoding/json"

var callSearBridge = callSearNative

func CallSear(requestJSON []byte, debug bool) (*NativeResponse, error) {
	return callSearBridge(requestJSON, debug)
}

func SearJSON(requestJSON []byte, debug bool) (*SecurityResult, error) {
	response, err := CallSear(requestJSON, debug)
	if err != nil {
		return nil, err
	}

	resultPayload := make(map[string]any)
	if len(response.ResultJSON) > 0 {
		if unmarshalErr := json.Unmarshal(response.ResultJSON, &resultPayload); unmarshalErr != nil {
			return nil, &JSONError{Cause: unmarshalErr}
		}
	}

	requestPayload := make(map[string]any)
	if len(requestJSON) > 0 {
		if unmarshalErr := json.Unmarshal(requestJSON, &requestPayload); unmarshalErr != nil {
			return nil, &RequestError{Cause: unmarshalErr}
		}
	}

	return &SecurityResult{
		Request:    requestPayload,
		RawRequest: response.RawRequest,
		RawResult:  response.RawResult,
		Result:     resultPayload,
	}, nil
}

func Sear(request map[string]any, debug bool) (*SecurityResult, error) {
	requestJSON, err := json.Marshal(request)
	if err != nil {
		return nil, &RequestError{Cause: err}
	}

	return SearJSON(requestJSON, debug)
}
