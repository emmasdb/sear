package main

import (
	"fmt"
	"log"

	"github.com/Mainframe-Renewal-Project/sear/go/sear"
)

func main() {
	request := map[string]any{
		"operation":  "extract",
		"admin_type": "user",
		"userid":     "IBMUSER",
	}

	result, err := sear.Sear(request, false)
	if err != nil {
		log.Fatalf("sear call failed: %v", err)
	}

	fmt.Printf("%v\n", result.Result)
}
