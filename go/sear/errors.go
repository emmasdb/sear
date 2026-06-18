package sear

import "fmt"

type NativeError struct {
	Message string
}

func (err *NativeError) Error() string {
	return err.Message
}

type JSONError struct {
	Cause error
}

func (err *JSONError) Error() string {
	return fmt.Sprintf("json processing failed: %v", err.Cause)
}

func (err *JSONError) Unwrap() error {
	return err.Cause
}

type RequestError struct {
	Cause error
}

func (err *RequestError) Error() string {
	return fmt.Sprintf("request processing failed: %v", err.Cause)
}

func (err *RequestError) Unwrap() error {
	return err.Cause
}
