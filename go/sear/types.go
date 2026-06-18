package sear

type SecurityResult struct {
	Request    map[string]any
	RawRequest []byte
	RawResult  []byte
	Result     map[string]any
}

type NativeResponse struct {
	RawRequest []byte
	RawResult  []byte
	ResultJSON []byte
}
