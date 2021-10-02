#!/bin/bash
script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
project_dir="${script_dir}/.."
out_dir="$project_dir/.provision"

echo "INFO: Welcome to the device provisioning scripts"


echo "INFO: Using profile ${AWS_PROFILE:-default} in region ${AWS_REGION:-default}"
sleep 5

echo "INFO: Creating output directory"
mkdir -p $out_dir && rm -Rf $out_dir/*

echo "INFO: Creating certificate and keys for device"
aws iot create-keys-and-certificate \
  --set-as-active \
  --certificate-pem-outfile $out_dir/certificate.pem \
  --public-key-outfile $out_dir/public_key.pem \
  --private-key-outfile $out_dir/private_key.pem

echo "INFO: Device material created successfully."
echo "INFO: Ready for cloud deployment"
