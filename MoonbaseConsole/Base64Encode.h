#ifndef __Base64Encode_H__
#define __Base64Encode_H__



void SecureBase64Encode(unsigned char *pby, int cby, char *psz, int cbszMax);
bool SecureBase64Decode(unsigned char *pby, int cby, char *psz);



#endif
