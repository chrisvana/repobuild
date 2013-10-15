// Copyright 2013
// Author: Christopher Van Arsdale

package main

import (
  "encoding/json"
  "log"
  "io/ioutil"
  "os"
)

type Input struct {
  Name string `json:"name"`
  Sources []string `json:"sources"`
}

type Output struct {
  Name string `json:"name"`
  GoSources []string `json:"go_sources"`
}

func main() {
  bytes, err := ioutil.ReadAll(os.Stdin)
  if err != nil {
    log.Fatal("Could not read input: ", err)
  }

  raw_input := make(map[string]Input)
  err = json.Unmarshal(bytes, &raw_input)
  if err != nil {
    log.Fatal("Could not parse json: ", err)
  }
  input := raw_input["simple_plugin"]
  if len(input.Name) == 0 {
    log.Fatal("Require component Name.")
  }

  var output Output
  output.Name = input.Name
  output.GoSources = input.Sources

  raw_output := make(map[string]Output)  
  raw_output["go_binary"] = output
  enc := json.NewEncoder(os.Stdout)
  if err := enc.Encode(&raw_output); err != nil {
    log.Fatal("Json encoding error: ", err)
  }
}
