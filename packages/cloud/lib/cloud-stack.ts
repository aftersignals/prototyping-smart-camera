import { Stack, StackProps } from 'aws-cdk-lib';
import { CfnPolicy, CfnPolicyPrincipalAttachment, CfnThing, CfnThingPrincipalAttachment } from 'aws-cdk-lib/lib/aws-iot';
import { IBucket } from 'aws-cdk-lib/lib/aws-s3';
import { Construct } from 'constructs';

export interface CloudProps extends StackProps {

  /**
   * Name of the bucket where to store pictures.
   * Use this parameter to share a bucket for several cameras.
   */
  PicturesBucketName?: string;

  /**
   * Prefix to use for pictures of this camera in the bucket.
   */
  PicturesPrefix?: string;

  /**
   * Arn of the existing Iot policy to use.
   */
  PolicyName?: string;

  /**
   * Arn of the certificate to attach the thing policy to
   */
  CertificateArn: string;

  /**
   * Name of the camera in the Iot footprint
   */
  ThingName: string;
}

export class CloudStack extends Stack {

  /**
   * Thing associated with this camera
   */
  public readonly deviceThing: CfnThing;

  /**
   * Bucket to store pictures from this device
   */
  public readonly picturesBucket: IBucket;

  /**
   * Policy for the device in AWS Iot
   */
  public readonly iotPolicy: CfnPolicy;

  /**
   * Link between the existing authentication material from the device and the cloud thing
   */
  public readonly certificateThingAttachment: CfnThingPrincipalAttachment;

  /**
   * Link between the certificate and an access policy
   */
  public readonly certificatePolicyAttachment: CfnPolicyPrincipalAttachment;

  constructor(scope: Construct, id: string, props: CloudProps) {
    super(scope, id, props);

    // Create device footprint
    this.deviceThing = new CfnThing(this, 'DeviceThing', {
      thingName: props.ThingName
    });

    // Attach the thing to the certificate
    this.certificateThingAttachment = new CfnThingPrincipalAttachment(this, 'CertificateThingAttachment', {
      principal: props.CertificateArn,
      thingName: this.deviceThing.ref
    });

    // Create Iot policy if it does not exist
    let policyName = props.PolicyName;
    if (!props.PolicyName) {
      this.iotPolicy = new CfnPolicy(this, 'IotPolicy', {
        policyDocument: {
          Version: "2012-10-17",
          Statement: [
            {
              Effect: 'Allow',
              Action: [
                'iot:Connect',
                'iot:Publish',
                'iot:Receive',
                'iot:Subscribe'
              ],
              Resource: [
                `*` // TODO
              ]
            }
          ]
        }
      });
      
      policyName = this.iotPolicy.ref;
    }

    // Attach policy to certificate
    this.certificatePolicyAttachment = new CfnPolicyPrincipalAttachment(this, 'PolicyPrincipalAttachment', {
      policyName: policyName!,
      principal: props.CertificateArn
    })

    // TODO bucket
  }
}
