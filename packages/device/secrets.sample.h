#include <pgmspace.h>

#define SECRET

// Amazon Root CA 1
// Get from https://docs.aws.amazon.com/iot/latest/developerguide/server-authentication.html#server-authentication-certs
static const char AWS_CERT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----

PASTE YOUR ROOT CA CONTENTS HERE

-----END CERTIFICATE-----
)EOF";

// Device Certificate
static const char AWS_CERT_CRT[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----

PASTE YOUR CERTIFICATE CONTENTS HERE

-----END CERTIFICATE-----
)KEY";

// Device Private Key
static const char AWS_CERT_PRIVATE[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----

PASTE YOUR PRIVATE KEY CONTENTS HERE

-----END RSA PRIVATE KEY-----
)KEY";