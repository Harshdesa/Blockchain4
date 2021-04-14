using namespace std;

#include <openssl/evp.h>
#include <openssl/pem.h>
#include<vector>
#include <iomanip>

#include <iostream>
#include <string>
#define BN_CHECK_BREAK(x)  if((x == NULL) || (BN_is_zero(x))){break;}
#ifndef NULL_BREAK
#define NULL_BREAK(x)   if(!x){break;}
#endif //NULL_BREAK
#define RSA_MOD_SIZE 384 //hardcode n size to be 384


int sgx_create_rsa_key_pair(int n_byte_size, int e_byte_size, unsigned char *p_n, unsigned char *p_d, unsigned char *p_e,
	unsigned char *p_p, unsigned char *p_q, unsigned char *p_dmp1,
	unsigned char *p_dmq1, unsigned char *p_iqmp)
{
	if (n_byte_size <= 0 || e_byte_size <= 0 || p_n == NULL || p_d == NULL || p_e == NULL ||
		p_p == NULL || p_q == NULL || p_dmp1 == NULL || p_dmq1 == NULL || p_iqmp == NULL) {
		return 0;
	}

	int ret_code = 0;
	RSA* rsa_ctx = NULL;
	BIGNUM* bn_n = NULL;
	BIGNUM* bn_e = NULL;
	BIGNUM* tmp_bn_e = NULL;
	BIGNUM* bn_d = NULL;
	BIGNUM* bn_dmp1 = NULL;
	BIGNUM* bn_dmq1 = NULL;
	BIGNUM* bn_iqmp = NULL;
	BIGNUM* bn_q = NULL;
	BIGNUM* bn_p = NULL;

	do {
		//create new rsa ctx
		//
		rsa_ctx = RSA_new();
		if (rsa_ctx == NULL) {
			ret_code = 0;
			break;
		}

		//generate rsa key pair, with n_byte_size*8 mod size and p_e exponent
		//
		tmp_bn_e = BN_lebin2bn(p_e, e_byte_size, tmp_bn_e);
		BN_CHECK_BREAK(tmp_bn_e);
		if (RSA_generate_key_ex(rsa_ctx, n_byte_size * 8, tmp_bn_e, NULL) != 1) {
			break;
		}

		//validate RSA key size match input parameter n size
		//
		int gen_rsa_size = RSA_size(rsa_ctx);
		if (gen_rsa_size != n_byte_size) {
			break;
		}

		//get RSA key internal values
		//
		RSA_get0_key(rsa_ctx, (const BIGNUM**)(&bn_n), (const BIGNUM**)(&bn_e), (const BIGNUM**)(&bn_d));
		RSA_get0_factors(rsa_ctx, (const BIGNUM**)(&bn_p), (const BIGNUM**)(&bn_q));
		RSA_get0_crt_params(rsa_ctx, (const BIGNUM**)(&bn_dmp1), (const BIGNUM**)(&bn_dmq1), (const BIGNUM**)(&bn_iqmp));

		//copy the generated key to input pointers
		//
		if (!BN_bn2lebinpad(bn_n, p_n, BN_num_bytes(bn_n)) ||
			!BN_bn2lebinpad(bn_d, p_d, BN_num_bytes(bn_d)) ||
			!BN_bn2lebinpad(bn_e, p_e, BN_num_bytes(bn_e)) ||
			!BN_bn2lebinpad(bn_p, p_p, BN_num_bytes(bn_p)) ||
			!BN_bn2lebinpad(bn_q, p_q, BN_num_bytes(bn_q)) ||
			!BN_bn2lebinpad(bn_dmp1, p_dmp1, BN_num_bytes(bn_dmp1)) ||
			!BN_bn2lebinpad(bn_dmq1, p_dmq1, BN_num_bytes(bn_dmq1)) ||
			!BN_bn2lebinpad(bn_iqmp, p_iqmp, BN_num_bytes(bn_iqmp))) {
			break;
		}

		ret_code = 1;
	} while (0);

	//free rsa ctx (RSA_free also free related BNs obtained in RSA_get functions)
	//
	RSA_free(rsa_ctx);
	BN_clear_free(tmp_bn_e);

	return ret_code;
}

int sgx_create_rsa_pub1_key(int mod_size, int exp_size, const unsigned char *le_n, const unsigned char *le_e, void **new_pub_key1)
{
	if (new_pub_key1 == NULL || mod_size <= 0 || exp_size <= 0 || le_n == NULL || le_e == NULL) {
		return 0;
	}

	EVP_PKEY *rsa_key = NULL;
	RSA *rsa_ctx = NULL;
	int ret_code = 0;
	BIGNUM* n = NULL;
	BIGNUM* e = NULL;

	do {
		//convert input buffers to BNs
		//
		n = BN_lebin2bn(le_n, mod_size, n);
		BN_CHECK_BREAK(n);
		e = BN_lebin2bn(le_e, exp_size, e);
		BN_CHECK_BREAK(e);

		// allocates and initializes an RSA key structure
		//
		rsa_ctx = RSA_new();
		rsa_key = EVP_PKEY_new();

		if (rsa_ctx == NULL || rsa_key == NULL || !EVP_PKEY_assign_RSA(rsa_key, rsa_ctx)) {
			RSA_free(rsa_ctx);
			rsa_ctx = NULL;
			break;
		}

		//set n, e values of RSA key
		//Calling set functions transfers the memory management of input BNs to the RSA object,
		//and therefore the values that have been passed in should not be freed by the caller after these functions has been called.
		//
		if (!RSA_set0_key(rsa_ctx, n, e, NULL)) {
			break;
		}
		*new_pub_key1 = rsa_key;
		ret_code = 1;
	} while (0);

	if (ret_code != 1) {
		EVP_PKEY_free(rsa_key);
		BN_clear_free(n);
		BN_clear_free(e);
	}

	return ret_code;
}

int sgx_create_rsa_priv2_key(int mod_size, int exp_size, const unsigned char *p_rsa_key_e, const unsigned char *p_rsa_key_p, const unsigned char *p_rsa_key_q,
	const unsigned char *p_rsa_key_dmp1, const unsigned char *p_rsa_key_dmq1, const unsigned char *p_rsa_key_iqmp,
	void **new_pri_key2)
{
	if (mod_size <= 0 || exp_size <= 0 || new_pri_key2 == NULL ||
		p_rsa_key_e == NULL || p_rsa_key_p == NULL || p_rsa_key_q == NULL || p_rsa_key_dmp1 == NULL ||
		p_rsa_key_dmq1 == NULL || p_rsa_key_iqmp == NULL) {
		return 0;
	}

	bool rsa_memory_manager = 0;
	EVP_PKEY *rsa_key = NULL;
	RSA *rsa_ctx = NULL;
	int ret_code = 0;
	BIGNUM* n = NULL;
	BIGNUM* e = NULL;
	BIGNUM* d = NULL;
	BIGNUM* dmp1 = NULL;
	BIGNUM* dmq1 = NULL;
	BIGNUM* iqmp = NULL;
	BIGNUM* q = NULL;
	BIGNUM* p = NULL;
	BN_CTX* tmp_ctx = NULL;

	do {
		tmp_ctx = BN_CTX_new();
		NULL_BREAK(tmp_ctx);
		n = BN_new();
		NULL_BREAK(n);

		// convert RSA params, factors to BNs
		//
		p = BN_lebin2bn(p_rsa_key_p, (mod_size / 2), p);
		BN_CHECK_BREAK(p);
		q = BN_lebin2bn(p_rsa_key_q, (mod_size / 2), q);
		BN_CHECK_BREAK(q);
		dmp1 = BN_lebin2bn(p_rsa_key_dmp1, (mod_size / 2), dmp1);
		BN_CHECK_BREAK(dmp1);
		dmq1 = BN_lebin2bn(p_rsa_key_dmq1, (mod_size / 2), dmq1);
		BN_CHECK_BREAK(dmq1);
		iqmp = BN_lebin2bn(p_rsa_key_iqmp, (mod_size / 2), iqmp);
		BN_CHECK_BREAK(iqmp);
		e = BN_lebin2bn(p_rsa_key_e, (exp_size), e);
		BN_CHECK_BREAK(e);

		// calculate n value
		//
		if (!BN_mul(n, p, q, tmp_ctx)) {
			break;
		}

		//calculate d value
		//ϕ(n)=(p−1)(q−1)
		//d=(e^−1) mod ϕ(n)
		//
		d = BN_dup(n);
		NULL_BREAK(d);

		//select algorithms with an execution time independent of the respective numbers, to avoid exposing sensitive information to timing side-channel attacks.
		//
		BN_set_flags(d, BN_FLG_CONSTTIME);
		BN_set_flags(e, BN_FLG_CONSTTIME);

		if (!BN_sub(d, d, p) || !BN_sub(d, d, q) || !BN_add_word(d, 1) || !BN_mod_inverse(d, e, d, tmp_ctx)) {
			break;
		}

		// allocates and initializes an RSA key structure
		//
		rsa_ctx = RSA_new();
		rsa_key = EVP_PKEY_new();

                //EVP_PKEY_assign_RSA() use the supplied key internally and so if this call succeed, key will be freed when the parent pkey is freed.
                //
		if (rsa_ctx == NULL || rsa_key == NULL || !EVP_PKEY_assign_RSA(rsa_key, rsa_ctx)) {
			RSA_free(rsa_ctx);
			rsa_key = NULL;
			break;
		}

		//setup RSA key with input values
		//Calling set functions transfers the memory management of the values to the RSA object,
		//and therefore the values that have been passed in should not be freed by the caller after these functions has been called.
		//
		if (!RSA_set0_factors(rsa_ctx, p, q)) {
			break;
		}
		rsa_memory_manager = 1;
		if (!RSA_set0_crt_params(rsa_ctx, dmp1, dmq1, iqmp)) {
			BN_clear_free(n);
			BN_clear_free(e);
			BN_clear_free(d);
			BN_clear_free(dmp1);
			BN_clear_free(dmq1);
			BN_clear_free(iqmp);
			break;
		}

		if (!RSA_set0_key(rsa_ctx, n, e, d)) {
			BN_clear_free(n);
			BN_clear_free(e);
			BN_clear_free(d);
			break;
		}

		*new_pri_key2 = rsa_key;
		ret_code = 1;
	} while (0);

	BN_CTX_free(tmp_ctx);

	//in case of failure, free allocated BNs and RSA struct
	//
	if (ret_code != 1) {
		//BNs were not assigned to rsa ctx yet, user code must free allocated BNs
		//
		if (!rsa_memory_manager) {
			BN_clear_free(n);
			BN_clear_free(e);
			BN_clear_free(d);
			BN_clear_free(dmp1);
			BN_clear_free(dmq1);
			BN_clear_free(iqmp);
			BN_clear_free(q);
			BN_clear_free(p);
		}
		EVP_PKEY_free(rsa_key);
	}

	return ret_code;
}

int main()
{
	void *public_key = NULL;
        void *private_key = NULL;

        unsigned char p_n[384], p_d[384], p_p[384], p_q[384], p_dmp1[384], p_dmq1[384], p_iqmp[384];
        long p_e = 65537;

        std::string s = "Test";

        if (sgx_create_rsa_key_pair(RSA_MOD_SIZE, sizeof(p_e), p_n, p_d, (unsigned char*)&p_e, p_p, p_q, p_dmp1, p_dmq1, p_iqmp) == 1){
                s = s + "Created key pair";
        }

        if(sgx_create_rsa_pub1_key(RSA_MOD_SIZE, sizeof(p_e), p_n, (unsigned char*)&p_e, &public_key) == 1) {
                s = s + "Reached Public Key phase";
        }

	if(sgx_create_rsa_priv2_key(RSA_MOD_SIZE, sizeof(p_e), (unsigned char*)&p_e, p_p, p_q, p_dmp1, p_dmq1, p_iqmp, &private_key) == 1) {
                s = s + "Reached Private Key phase";
        }

	//cout << s;
	//char* pChar;
	//pChar = (char*)public_key;
	//s = s + *pChar;
        //while (*pChar != NULL) {
        //      s = s + *pChar;
        //      pChar++;
       // }
	//std::string someString(pChar);
	//cout << someString;
	
	/* EVP_PKEY_DERIVE METHOD*/
	/*EVP_PKEY_CTX *ctxr = NULL;
        ctxr = EVP_PKEY_CTX_new((EVP_PKEY*)public_key, NULL);
        size_t test_len = 0;
        if(EVP_PKEY_derive(ctxr, NULL, &test_len)) {
              s = s + "Success";
        }
        uint8_t test_key[test_len];
        if(EVP_PKEY_derive(ctxr, test_key, &test_len)) {
              s = s + "success";
        }
	EVP_PKEY_CTX_free(ctxr);
	char* test_key_as_char = reinterpret_cast<char*>(test_key);
        int d;
        for (d = 0; d < test_len; d++)
        {
              s.append(1, test_key_as_char[d]);
        }
	cout << "USING EVP_PKEY_derive PART 1";
	cout << s;
	printf("\"%s\"\n", test_key_as_char);
	*/

	/*i2d_publickey METHOD*/
	EVP_PKEY *pkey = (EVP_PKEY*)public_key;
        if (public_key == NULL) {
                return 0;
         }

        std::string s2 = "Test";
	uint8_t *ucBuf;

        //uint8_t *output = NULL;
        size_t pkeyLen = i2d_PublicKey(pkey, NULL);
	std::vector<unsigned char> buf(pkeyLen, 0x00);
	unsigned char *output = &buf[0];
	//output = (uint8_t *)malloc(pkeyLen+1);
        pkeyLen = i2d_PublicKey(pkey, &output);
	std::stringstream ssResult;
	ssResult << std::hex;

	for(auto value: buf)
        {
            ssResult << std::setw(2) << std::setfill('0') << (int) (value);
	    //ssResult << (int) (value);
        }

	std::string result = ssResult.str();
	cout << result;
	std::cout << "END - END -END- PUBLIC KEY" << std::endl;
	ucBuf = output;

        int c;
        for (c = 0; c < pkeyLen; c++)
        {
		s2.append(1, (unsigned char) output[c]);
        }
	std::cout << s2 << std::endl;
	std::cout << "END" << std::endl;

	/*i2d_privatekey METHOD*/
        EVP_PKEY *prkey = (EVP_PKEY*)private_key;
        if (private_key == NULL) {
                return 0;
         }

        std::string s3 = "Test";

        //uint8_t *output = NULL;
        size_t prkeyLen = i2d_PrivateKey(prkey, NULL);
        std::vector<unsigned char> bufr(prkeyLen, 0x00);
        unsigned char *outputr = &bufr[0];
        //output = (uint8_t *)malloc(pkeyLen+1);
        prkeyLen = i2d_PrivateKey(prkey, &outputr);
        std::stringstream ssrResult;
        ssrResult << std::hex;

        for(auto value: bufr)
	{
            ssrResult << std::setw(2) << std::setfill('0') << (int) (value);
            //ssResult << (int) (value);
        }

        std::string resultr = ssrResult.str();
        cout << resultr;
	std::cout << "END - END -END- PRIVATE KEY" << std::endl;
        for (c = 0; c < prkeyLen; c++)
        {
                s3.append(1, (unsigned char) outputr[c]);
        }
        std::cout << s3 << std::endl;
        std::cout << "END" << std::endl;


	/*BIO METHOD */
	//BIO *bp = BIO_new_fp(stdout, BIO_NOCLOSE);
	BIO *bp = BIO_new_fd(1,1);
	//BIO* bp = BIO_new(BIO_s_mem());
	//if(!EVP_PKEY_print_public(bp, pkey, 1, NULL))
   	//{
      	//	std::cout << "error 5" << std::endl;
   	//}
	//std::cout << "PUBLIC KEY" <<std::endl;
       	//std::cout << bp <<std::endl;
	//std::cout << "END" <<std::endl;
	//cout << s;
	BIO_free(bp);
        EVP_PKEY_free(pkey);


}


//int main()
//{
//
//    // create private/public key pair
//    // init RSA context, so we can generate a key pair
//    EVP_PKEY_CTX *keyCtx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
//    EVP_PKEY_keygen_init(keyCtx);
//    EVP_PKEY_CTX_set_rsa_keygen_bits(keyCtx, 4096); // RSA 4096
//    // variable that will hold both private and public keys
//    EVP_PKEY *key = NULL;
//    // generate key
//    EVP_PKEY_keygen(keyCtx, &key);
//    // free up key context
//    EVP_PKEY_CTX_free(keyCtx);
// 
//    
//    // extract private key as string
//    // create a place to dump the IO, in this case in memory
//    BIO *privateBIO = BIO_new(BIO_s_mem());
//    // dump key to IO
//    PEM_write_bio_PrivateKey(privateBIO, key, NULL, NULL, 0, 0, NULL);
//    // get buffer length
//    int privateKeyLen = BIO_pending(privateBIO);
//    // create char reference of private key length
//    unsigned char *privateKeyChar = (unsigned char *) malloc(privateKeyLen);
//    // read the key from the buffer and put it in the char reference
//    BIO_read(privateBIO, privateKeyChar, privateKeyLen);
//    // at this point we can save the private key somewhere
//    
//    
//    // extract public key as string
//    // create a place to dump the IO, in this case in memory
//    BIO *publicBIO = BIO_new(BIO_s_mem());
//    // dump key to IO
//    PEM_write_bio_PUBKEY(publicBIO, key);
//    // get buffer length
//    int publicKeyLen = BIO_pending(publicBIO);
//    // create char reference of public key length
//    unsigned char *publicKeyChar = (unsigned char *) malloc(publicKeyLen);
//    // read the key from the buffer and put it in the char reference
//    BIO_read(publicBIO, publicKeyChar, publicKeyLen);
//    // at this point we can save the public somewhere
//
//    
//    // pretend we are pulling the public key from some source and using it
//    // to encrypt a message
//    unsigned char *rsaPublicKeyChar = publicKeyChar;
//    // write char array to BIO
//    BIO *rsaPublicBIO = BIO_new_mem_buf(rsaPublicKeyChar, -1);
//    // create a RSA object from public key char array
//    RSA *rsaPublicKey = NULL;
//    PEM_read_bio_RSA_PUBKEY(rsaPublicBIO, &rsaPublicKey, NULL, NULL);
//    // create public key
//    EVP_PKEY *publicKey = EVP_PKEY_new();
//    EVP_PKEY_assign_RSA(publicKey, rsaPublicKey);
//    // initialize encrypt context
//    EVP_CIPHER_CTX *rsaEncryptCtx = (EVP_CIPHER_CTX *) malloc(10000);
//    EVP_CIPHER_CTX_init(rsaEncryptCtx);
//    // variables for where the encrypted secret, length, and IV reside
//    unsigned char *ek = (unsigned char *) malloc(EVP_PKEY_size(publicKey));
//    int ekLen = 0;
//    unsigned char *iv = (unsigned char *) malloc(EVP_MAX_IV_LENGTH);
//    // generate AES secret, and encrypt it with public key
//    EVP_SealInit(rsaEncryptCtx, EVP_aes_256_cbc(), &ek, &ekLen, iv, &publicKey, 1);
//    // encrypt a message with AES secret
//    string message = "You can include the standard headers in any order, a standard header more than once, or two or more standard headers that define the same macro or the same type. Do not include a standard header within a declaration. Do not define macros that have the same names as keywords before you include a standard header.";
//    const unsigned char* messageChar = (const unsigned char*) message.c_str();
//    // length of message
//    int messageLen = message.size() + 1;
//    // create char reference for where the encrypted message will reside
//    unsigned char *encryptedMessage = (unsigned char *) malloc(messageLen + EVP_MAX_IV_LENGTH);
//    // the length of the encrypted message
//    int encryptedMessageLen = 0;
//    int encryptedBlockLen = 0;
//    // encrypt message with AES secret
//    EVP_SealUpdate(rsaEncryptCtx, encryptedMessage, &encryptedBlockLen, messageChar, messageLen);
//    encryptedMessageLen = encryptedBlockLen;
//    // finalize by encrypting the padding
//    EVP_SealFinal(rsaEncryptCtx, encryptedMessage + encryptedBlockLen, &encryptedBlockLen);
//    encryptedMessageLen += encryptedBlockLen;
//    
//    
//    // pretend we are decrypting a message we have received using a the private key we have
//    unsigned char *rsaPrivateKeyChar = privateKeyChar;
//    // write char array to BIO
//    BIO *rsaPrivateBIO = BIO_new_mem_buf(rsaPrivateKeyChar, -1);
//    // create a RSA object from private key char array
//    RSA *rsaPrivateKey = NULL;
//    PEM_read_bio_RSAPrivateKey(rsaPrivateBIO, &rsaPrivateKey, NULL, NULL);
//    // create private key
//    EVP_PKEY *privateKey = EVP_PKEY_new();
//    EVP_PKEY_assign_RSA(privateKey, rsaPrivateKey);
//    // initialize decrypt context
//    EVP_CIPHER_CTX *rsaDecryptCtx = (EVP_CIPHER_CTX *) malloc(10000);
//    EVP_CIPHER_CTX_init(rsaDecryptCtx);
//    // decrypt EK with private key, and get AES secretp
//    EVP_OpenInit(rsaDecryptCtx, EVP_aes_256_cbc(), ek, ekLen, iv, privateKey);
//    // variable for where the decrypted message with be outputed to
//    unsigned char *decryptedMessage = (unsigned char *) malloc(encryptedMessageLen + EVP_MAX_IV_LENGTH);
//    // the length of the encrypted message
//    int decryptedMessageLen = 0;
//    int decryptedBlockLen = 0;
//    // decrypt message with AES secret
//    EVP_OpenUpdate(rsaDecryptCtx, decryptedMessage, &decryptedBlockLen, encryptedMessage, encryptedMessageLen);
//    decryptedMessageLen = decryptedBlockLen;
//    // finalize by decrypting padding
//    EVP_OpenFinal(rsaDecryptCtx, decryptedMessage + decryptedBlockLen, &decryptedBlockLen);
//    decryptedMessageLen += decryptedBlockLen;
//    
//printf("%s\n", encryptedMessage);
//printf("\"%s\"\n", decryptedMessage);
//    
//}
