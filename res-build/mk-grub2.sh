#!/bin/bash -e

. "`dirname $0`/vars.sh"

if [[ ! -e "$key" || ! -e "$crt" ]]; then
    echo "Create keys first"
    exit 1
fi

unsigned="$res/grub2x64-unsigned.efi"
signed="$res/grub2x64.efi"

grub-mkstandalone \
    -O x86_64-efi \
    -o "$unsigned" \
    --locales= \
    --compress=gz \
    /boot/grub/grub.cfg="$res_build/grub2.cfg"

sbsign \
    --key "$key" --cert "$crt" \
    --output "$signed" \
    "$unsigned"

rm "$unsigned"

tmp="`mktemp`"
openssl x509 \
    -in "$cer_res" -inform der \
    -out "$tmp"

sbverify \
    --cert "$tmp" "$signed"

rm "$tmp"

echo "Success"
