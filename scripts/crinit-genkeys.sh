#!/bin/sh -e
# SPDX-License-Identifier: MIT

OUTPUT_FILE="-"
OUTPUT_FORM="pem"
KEY_FILE=""

print_help() {
    cat <<EOF
Usage: $0 [-h/--help] [-k/--key-file <KEY_FILE>] [-o/--output <OUTPUT_FILE>]
  If no other arguments are given, will generate an RSA-4096 private key. Alternatively using '-k', you can obtain
  the public key for the given private key.
    -h/--help
        - Show this help.
    -f/--format
        - Choose the format of the output file. Must be either 'pem' or 'der'. Default: 'pem'
    -k/--key-file <KEY_FILE>
        - Generate a public key from the given private key. Use '-' for Standard Input. Default: Generate a private key.
    -o/--output <OUTPUT_FILE>
        - The filename of the output key. Default: write to Standard Output
EOF
}

while :; do
    case "$1" in
        -h | --help)
            print_help
            exit 1
            ;;
        -k | --key-file)
            if [ -n "$2" ]; then
                KEY_FILE="$2"
                shift
            fi
            ;;
        -o | --output)
            if [ -n "$2" ]; then
                OUTPUT_FILE="$2"
                shift
            fi
            ;;
        -f | --format)
            if [ -n "$2" ]; then
                OUTPUT_FORM="$2"
                shift
            fi
            ;;
        -?*)
            echo "Unknown option encountered: $1" 1>&2
            print_help
            exit 1
            ;;
        *)
            break
            ;;
    esac
    shift
done

# If a private key is input, output the public key counterpart.
if [ -n "${KEY_FILE}" ]; then
    if [ ! -e "${KEY_FILE}" ]; then
        echo "The given path to the key file ('${KEY_FILE}') is invalid." 1>&2
        exit 1
    fi
    openssl rsa -out "${OUTPUT_FILE}" -outform "${OUTPUT_FORM}" -pubout <"${KEY_FILE}"
    exit
fi

# Otherwise, generate a new private key.
openssl genpkey -algorithm rsa -outform "${OUTPUT_FORM}" -pkeyopt rsa_keygen_bits:4096 -out "${OUTPUT_FILE}"
