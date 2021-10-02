#!/usr/bin/env node
import 'source-map-support/register';
import * as cdk from 'aws-cdk-lib';
import { CloudStack } from '../lib/cloud-stack';

const app = new cdk.App();
new CloudStack(app, 'CloudStack', {
  CertificateArn: 'YOUR_CERTIFICATE_ARN_HERE',
  ThingName: 'Camera1',
});