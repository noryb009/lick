#!/bin/bash -e

. "`dirname $0`/vars.sh"

if [[ $# -eq 0 ]]; then
    echo "Usage: $0 key-comment"
    exit 1
fi

mkdir -p "$keys"

if [[ -e "$key" || -e "$crt" || -e "$cer" ]]; then
    echo "Key already exists"
    exit 1
fi

openssl \
    req -new -x509 -newkey rsa:2048 \
    -keyout "$key" -out "$crt" \
    -nodes -days 3650 \
    -subj "/CN=$1/"
openssl x509 \
    -in "$crt" -out "$cer" \
    -outform DER
cp "$cer" "$cer_res"

chmod 400 "$key" "$crt" "$cer"

echo "Success"
