//go:build !sear_native

package sear

func callSearNative(_ []byte, _ bool) (*NativeResponse, error) {
	return nil, &NativeError{Message: "native SEAR bridge is disabled; build with -tags sear_native"}
}
