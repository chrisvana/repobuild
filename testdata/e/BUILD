[
{ "cc_library": {
    "name": "print-build-timestamp",
    "ephemeral_cc_sources": [ "print-build-timestamp.c" ],
    "cc_compile_args": [ "-DBUILD_TIMESTAMP=\\\"\"$(date --rfc-3339=seconds)\"\\\"" ]
} },
{ "cc_binary": {
    "name": "main",
    "cc_sources": [ "main.c" ],
    "dependencies": [ ":print-build-timestamp" ]
} }
]