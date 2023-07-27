#!/bin/sh -e

INPUT_FILE="-"
OUTPUT_FILE="-"
KEY_FILE=""

print_help() {
    cat <<EOF
Usage: $0 [-h/--help] [-k/--key-file <KEY_FILE>] [-o/--output <OUTPUT_FILE>] [<INPUT_FILE>]
  Will sign given input data with an RSA-PSS signature from an RSA-4096 private key using a SHA-256 hash.
    -h/--help
        - Show this help.
    -k/--key-file <KEY_FILE>
        - Use the given private key to sign. Must have been created using crinit-genkeys.sh. (Mandatory)
    -o/--output <OUTPUT_FILE>
        - Write signature to OUTPUT_FILE. Default: Standard Output
    <INPUT_FILE>
        - Positional argument. The input data to sign. Default: Standard Input
EOF
}

while :; do
    case "$1" in
        -h|--help)
            print_help
            exit 1
            ;;
        -k|--key-file)
            if [ -n "$2" ]; then
                KEY_FILE="$2"
                shift
            fi
            ;;
        -o|--output)
            if [ -n "$2" ]; then
                OUTPUT_FILE="$2"
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

if [ -n "$1" ]; then
    INPUT_FILE="$1"
fi

if [ ! -e "${KEY_FILE}" ]; then
    echo "The given path to the key file ('${KEY_FILE}') is invalid." 1>&2
    exit 1
fi

# check key consistency and type (RSA-4096)
OSSL_KEYCHECK=$(openssl rsa -text -noout -check -in "${KEY_FILE}")

RETURNCODE=0
echo "${OSSL_KEYCHECK}" | grep -q "RSA key ok" || RETURNCODE=$?
if [ ${RETURNCODE} -ne 0 ]; then
    echo "The given key is not a valid RSA key." 1>&2
    exit 1
fi

echo "${OSSL_KEYCHECK}" | grep -q "Private-Key: (4096 bit, 2 primes)" || RETURNCODE=$?
if [ ${RETURNCODE} -ne 0 ]; then
    echo "The given key is not an RSA-4096 key." 1>&2
    exit 1
fi

openssl dgst -sha256 -sigopt rsa_padding_mode:pss -sigopt rsa_pss_saltlen:-1 -sigopt rsa_mgf1_md:sha256 -sign "${KEY_FILE}" -out "${OUTPUT_FILE}" "${INPUT_FILE}"
