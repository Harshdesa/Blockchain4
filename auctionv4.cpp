using namespace std;
#include "shim.h"
#include <map>
#include <sstream>
#include "logging.h"
#include <string>
#include <ctime>
#include <cstring>
//#include <stdio.h>
#include <iostream>
#include <ostream>
#include<vector>
#include <iomanip>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <math.h>
#include <random>
#include "sgx_urts.h"
#include "sgx_tcrypto.h"
#include "sgx_utils.h"
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <UniquePtr.h>

#define OK "OK"
#define NOT_FOUND "Bid not found"
#define RSA_MOD_SIZE 384 //hardcode n size to be 384
#define RSA_E_SIZE 4 //hardcode e size to be 4

#define MAX_VALUE_SIZE 1024
int user_count = 0;
std::map<int, std::string> usernames;
std::map<int, std::string> usernamesB;
int user_countB = 0;
std::map<std::string, std::string> userPublicKeysTest;
void *chaincode_public_key = NULL;
void *chaincode_private_key = NULL;
sgx_rsa3072_key_t chaincode_signing_private_key;
sgx_rsa3072_public_key_t chaincode_signing_public_key;

std::string testVariable;
std::map<std::string, void*> userPrivateKeys;
std::map<std::string, void*> userPublicKeys;
std::map<std::string, sgx_rsa3072_key_t> userSigningPrivateKeys;
std::map<std::string, sgx_rsa3072_public_key_t> userSigningPublicKeys;
sgx_rsa3072_signature_t cache_signature[384];
unsigned char *user_encrypted_data;
const uint8_t *cache_signed_data;
size_t cache_data_len = 0;

std::string putTestVariable(std::string a, shim_ctx_ptr_t ctx)
{
	testVariable = a;
	return testVariable;
}

std::string getTestVariable(shim_ctx_ptr_t ctx)
{
        return testVariable;
}


//Will store the public key and return private key for user
//Called ONLY BY User
std::string  createUserPublicPrivateKey(std::string user_name, shim_ctx_ptr_t ctx)	
{
	
	void *public_key = NULL;
	void *private_key = NULL;
	
	unsigned char p_n[384], p_d[384], p_p[384], p_q[384], p_dmp1[384], p_dmq1[384], p_iqmp[384]; 
	long p_e = 65537;

	std::string s = "Test";

	if (sgx_create_rsa_key_pair(RSA_MOD_SIZE, sizeof(p_e), p_n, p_d, (unsigned char*)&p_e, p_p, p_q, p_dmp1, p_dmq1, p_iqmp) == SGX_SUCCESS){ 
		s = s + "Created key pair";
	}
	sgx_rsa3072_key_t rsa_key;
	unsigned char temp_n[384], temp_d[384];
	long temp_e;
	for(int i = 0; i<384; i++) {
		temp_n[i] = p_n[i];
		temp_d[i] = p_d[i];
	}
	temp_e = p_e;
        memcpy(&(rsa_key.mod), &(temp_n), sizeof(rsa_key.mod));
        memcpy(&(rsa_key.d), &(temp_d), sizeof(rsa_key.d));
        memcpy(&(rsa_key.e), &(temp_e), sizeof(rsa_key.e));

        sgx_rsa3072_public_key_t temp_public_key;

        memcpy(&(temp_public_key.mod), &(temp_n), sizeof(temp_public_key.mod));
        memcpy(&(temp_public_key.exp), &(temp_e), sizeof(temp_public_key.exp));

        userSigningPrivateKeys[user_name] = rsa_key;
        userSigningPublicKeys[user_name] = temp_public_key;

	if(sgx_create_rsa_pub1_key(RSA_MOD_SIZE, sizeof(p_e), p_n, (unsigned char*)&p_e, &public_key) == SGX_SUCCESS) {
		s = s + "Reached Public Key phase";
	}

	if(sgx_create_rsa_priv2_key(RSA_MOD_SIZE, sizeof(p_e), (unsigned char*)&p_e, p_p, p_q, p_dmp1, p_dmq1, p_iqmp, &private_key) == SGX_SUCCESS) {
		s = s + "Reached Private Key phase";
	}
	
	userPublicKeys[user_name] = public_key;
	userPrivateKeys[user_name] = private_key;



	return s;
}


// To be called once at the beginning. Creates chaincode public private key.
std::string  createChaincodePublicPrivateKey(shim_ctx_ptr_t ctx)	
{
	unsigned char p_n[384], p_d[384], p_p[384], p_q[384], p_dmp1[384], p_dmq1[384], p_iqmp[384]; 
	long p_e = 65537;

	std::string s = "Test";

	if (sgx_create_rsa_key_pair(RSA_MOD_SIZE, sizeof(p_e), p_n, p_d, (unsigned char*)&p_e, p_p, p_q, p_dmp1, p_dmq1, p_iqmp) == SGX_SUCCESS){ 
		s = s + "Created key pair";
	}

	unsigned char temp_n[384], temp_d[384];
        long temp_e;
        for(int i = 0; i<384; i++) {
                temp_n[i] = p_n[i];
                temp_d[i] = p_d[i];
        }
        temp_e = p_e;

	memcpy(&(chaincode_signing_private_key.mod), &(temp_n), sizeof(chaincode_signing_private_key.mod));
        memcpy(&(chaincode_signing_private_key.d), &(temp_d), sizeof(chaincode_signing_private_key.d));
        memcpy(&(chaincode_signing_private_key.e), &(temp_e), sizeof(chaincode_signing_private_key.e));

        memcpy(&(chaincode_signing_public_key.mod), &(temp_n), sizeof(chaincode_signing_public_key.mod));
        memcpy(&(chaincode_signing_public_key.exp), &(temp_e), sizeof(chaincode_signing_public_key.exp));

	if(sgx_create_rsa_pub1_key(RSA_MOD_SIZE, sizeof(p_e), p_n, (unsigned char*)&p_e, &chaincode_public_key) == SGX_SUCCESS) {
		s = s + "Reached Public Key phase";
	}

	if(sgx_create_rsa_priv2_key(RSA_MOD_SIZE, sizeof(p_e), (unsigned char*)&p_e, p_p, p_q, p_dmp1, p_dmq1, p_iqmp, &chaincode_private_key) == SGX_SUCCESS) {
		s = s + "Reached Private Key phase";
	}

	char *pp = reinterpret_cast<char*>(p_p);
	char *pq = reinterpret_cast<char*>(p_q);
	char *pn = reinterpret_cast<char*>(p_n);


	int c = 0;
	s = s + "This is p_n";
        while(c < 384) {
                s.append(1, pn[c]);
                ++c;
        }

	c = 0;
        s = s + "This is p_p";
        while(c < 384) {
                s.append(1, pp[c]);
                ++c;
        }

	c = 0;
        s = s + "This is p_q";
        while(c < 384) {
                s.append(1, pq[c]);
                ++c;
        }


	s = s+ "This is p_d";
	c = 0;
        while(c < 384) {
                s.append(1, p_d[c]);
                ++c;
        }
	s = s+ "This is p_e";
	s = s + std::to_string(p_e);


	return s;
}

// To be called to retrieve the chaincode public key
std::string  retrieveChaincodePublicKey(shim_ctx_ptr_t ctx)	
{
	std::string s = "Test";
	EVP_PKEY_CTX *ctxr = NULL;
	if (chaincode_public_key == NULL) {
		return "error";
	 }
	EVP_PKEY * pkey = (EVP_PKEY*)chaincode_public_key;
        //ctxr = EVP_PKEY_CTX_new((EVP_PKEY*)chaincode_public_key, NULL);
	
	//BIO *out = NULL;
	//BIO_printf(out, "Netscape SPKI:\n");


	//EVP_PKEY_print_public(out, pkey, 16, NULL);
	//BIO *bp = BIO_new_fd(1, 1);
        //if(!EVP_PKEY_print_public(bp, pkey, 1, NULL))
        //{
	//	return "error";
        //}
	//s = s + bp;
        //BIO_free(bp);


	EVP_PKEY_free(pkey);

        //size_t test_len = 0;
        //if(EVP_PKEY_derive(ctxr, NULL, &test_len)) {
        //      s = s + "Success";
        //}
        //uint8_t test_key[test_len];
        //if(EVP_PKEY_derive(ctxr, test_key, &test_len)) {
        //      s = s + "success";
        //}
        EVP_PKEY_CTX_free(ctxr);
        //char* test_key_as_char = reinterpret_cast<char*>(test_key);
        //int d;
        //for (d = 0; d < 4; d++)
        //{
        //      s.append(1, test_key_as_char[d]);
        //}
	//
	//
	//

	//std::string pubKey = "";
	//EVP_PKEY* pkey = (EVP_PKEY*)chaincode_public_key;
	//BIO *bio = NULL;
    	//char *pem = NULL;
	//int pkeyLen;
	//unsigned char *ucBuf, *uctempBuf;
	//pkeyLen = i2d_PublicKey(pkey, NULL);
	//pkeyLen = i2d_PublicKey(pkey, &uctempBuf);
	//ucBuf = (unsigned char *)malloc(pkeyLen+1);
	//uctempBuf = ucBuf;
	//pubKey = pubKey + "hello" + std::to_string(pkeyLen);;

	//int ii = 0;
	//while(uctempBuf[ii] !=  NULL) {
	//	pubKey.append(1, (unsigned char) uctempBuf[ii]);
	//		++ii;
	//}

//	unsigned char* charpublicKey;
//	BIO* bio = BIO_new(BIO_s_mem());
//	PEM_write_bio_PUBKEY(bio, pkey);
//	RSAmakeString(&charpublicKey, bio);
//	BIO* bp = NULL;
//	if(!EVP_PKEY_print_public(bp, pkey, 1, NULL))
//	{
//		pubKey = pubKey + "Error";
//	}
//	else {
//		pubKey = pubKey + "It works";
//	}
//	int c = 0;
//        while(charpublicKey[c] != NULL) {
//                pubKey.append(1, charpublicKey[c]);
//                ++c;
//        }
//
//
//	BIO_free(bio);
//	char* pChar;
//	pChar = (char*)chaincode_public_key;
//	pubKey = pubKey + *pChar;
//	while (*pChar != NULL) {
//		pubKey = pubKey + *pChar;
//		pChar++;
//	}
//	std::string someString(pChar);
//	pubKey = pubKey + someString;

	//std::string pk_string(*(const char*)chaincode_public_key, 1000);
	//pubKey = pubKey + pk_string;

	//int c = 0;
        //while(pChar[c] != NULL) {
        //       	pubKey.append(1, pChar[c]);
	//	++c;
        //}
	return s;
}

//Example signing simulation
std::string signingSimulation(shim_ctx_ptr_t ctx)
{
	unsigned char p_n[384], p_d[384], p_p[384], p_q[384], p_dmp1[384], p_dmq1[384], p_iqmp[384];
	long p_e = 65537;
	sgx_rsa3072_signature_t p_signature[384];

	std::string str = "Hello, world";

	//uint8_t* p_data = reinterpret_cast<uint8_t*>(str.c_str());

	const uint8_t p_data[] = "Hello world";

	std::string s = "Test";
	if (sgx_create_rsa_key_pair(384, sizeof(p_e), p_n, p_d, (unsigned char*)&p_e, p_p, p_q, p_dmp1, p_dmq1, p_iqmp) == SGX_SUCCESS){
                s = s + "Created key pair";
        }

	sgx_rsa3072_key_t rsa_key;
	memcpy(&(rsa_key.mod), &(p_n), sizeof(rsa_key.mod));
	memcpy(&(rsa_key.d), &(p_d), sizeof(rsa_key.d));
	memcpy(&(rsa_key.e), &(p_e), sizeof(rsa_key.e));

	if (sgx_rsa3072_sign(p_data, sizeof(p_data), &rsa_key, p_signature) == SGX_SUCCESS) {
		s = s + "Signed data";
	}

	sgx_rsa3072_public_key_t temp_public_key;
	
	memcpy(&(temp_public_key.mod), &(p_n), sizeof(temp_public_key.mod));
	memcpy(&(temp_public_key.exp), &(p_e), sizeof(temp_public_key.exp));

	sgx_rsa_result_t verify_result = SGX_RSA_INVALID_SIGNATURE;

	if (sgx_rsa3072_verify(p_data, sizeof(p_data), &temp_public_key, p_signature, &verify_result) == SGX_SUCCESS){
		s = s + "Verified signature";	
	}

	if (verify_result == SGX_RSA_VALID)
    	{
        	s = s + "Valid Result";
    	}


	return s;
}

//Example encryption simulation
std::string  encryptionSimulation(shim_ctx_ptr_t ctx)
{
	void *public_key = NULL;
	void *private_key = NULL;
	
	unsigned char p_n[384], p_d[384], p_p[384], p_q[384], p_dmp1[384], p_dmq1[384], p_iqmp[384]; 
	long p_e = 65537;

	std::string s = "Test";

	if (sgx_create_rsa_key_pair(RSA_MOD_SIZE, sizeof(p_e), p_n, p_d, (unsigned char*)&p_e, p_p, p_q, p_dmp1, p_dmq1, p_iqmp) == SGX_SUCCESS){ 
		s = s + "Created key pair";
	}

	if(sgx_create_rsa_pub1_key(RSA_MOD_SIZE, sizeof(p_e), p_n, (unsigned char*)&p_e, &public_key) == SGX_SUCCESS) {
		s = s + "Reached Public Key phase";
	}

	if(sgx_create_rsa_priv2_key(RSA_MOD_SIZE, sizeof(p_e), (unsigned char*)&p_e, p_p, p_q, p_dmp1, p_dmq1, p_iqmp, &private_key) == SGX_SUCCESS) {
		s = s + "Reached Private Key phase";
	}	

	EVP_PKEY *pkey = (EVP_PKEY*)public_key;
        if (public_key == NULL) {
                return 0;
        }
	
	int pkeyLen;
	unsigned char *ucBuf, *uctempBuf;
	pkeyLen = i2d_PublicKey(pkey, NULL);
	ucBuf = (unsigned char *)malloc(pkeyLen+1);
	uctempBuf = ucBuf;
	if(i2d_PublicKey(pkey, &ucBuf) != pkeyLen) {
		return "does not work";
	}
	if (pkeyLen <= 0) {
		return "len is 0";
	}
	int ii;
	for (ii = 0; ii < pkeyLen; ii++)
	{
		s += static_cast<char>((unsigned char) ucBuf[ii]);
		s = s + "test";
		//printf("%02x\n", (unsigned char) ucBuf[ii]);
	}
	//int publicLen = i2d_PublicKey(pkey, NULL);
	//size_t* keyBlobLength;
        //*keyBlobLength = sizeof(int) + sizeof(int) + sizeof(int) + publicLen;
	//UniquePtr<unsigned char[]> derData(new unsigned char[*keyBlobLength]);
	//
	//if (derData.get() == NULL) {
    	//	return "derData.get failed";
	//}
	//int type = 0;
	//unsigned char* p = derData.get();
	//for (int i = sizeof(int) - 1; i >= 0; i--) {
        //	*p++ = (type >> (8*i)) & 0xFF;
    	//}
	//for (int i = sizeof(int) - 1; i >= 0; i--) {
        //	*p++ = (publicLen >> (8*i)) & 0xFF;
    	//}
	//if (i2d_PublicKey(pkey, &p) != publicLen) {
	//	return "This failed";
	//}
	//std::vector<unsigned char> buf(pkeyLen, 0x00);
	//unsigned char *buf,*ucBuf, *uctempBuf;
	//ucBuf = (unsigned char *)malloc(pkeyLen+1);
        //uctempBuf = ucBuf;
	//i2d_PublicKey(pkey, &uctempBuf);
        ////std::stringstream ssResult;
        ////ssResult << std::hex;
	//int ii = 0;
	//std::string sName(reinterpret_cast<char*>(uctempBuf));
	//s = s + sName;
	//for (ii = 0; ii < pkeyLen; ii++)
        //{
            //ssResult << std::setw(2) << std::setfill('0') << (int) (value);
            //s = s + (int) (value);
	    //s += boost::numeric_cast<char>(ucBuf[ii]);
	    //s.append(1, (int) (ucBuf[ii]));
	    //ssResult << (int) (value);
        //}

	//std::string result = ssResult.str();

	//s = s + result;

	s = s + "END = END = END ";

	//EVP_PKEY *prkey = (EVP_PKEY*)private_key;
        //if (private_key == NULL) {
        //        return 0;
        // }

	//size_t prkeyLen = i2d_PrivateKey(prkey, NULL);
        //std::vector<unsigned char> bufr(prkeyLen, 0x00);
        //unsigned char *outputr = &bufr[0];

	//prkeyLen = i2d_PrivateKey(prkey, &outputr);
        //std::stringstream ssrResult;
        //ssrResult << std::hex;

	//for(auto value: bufr)
        //{
            //ssrResult << std::setw(2) << std::setfill('0') << (int) (value);
	    //s.append(1, (int) (value));
	    //ssResult << (int) (value);
        //}

	//std::string resultr = ssrResult.str();
	
	//s = s + resultr;
	s = s + "END = END = END ";

	size_t pout_len = 0;

	char * pin_data = "Hello World!";

	if(sgx_rsa_pub_encrypt_sha256(public_key, NULL, &pout_len, (unsigned char *)pin_data, strlen(pin_data)) == SGX_SUCCESS) {
		s = s + "Encrypted";
	}

	unsigned char pout_data[pout_len];

	if(sgx_rsa_pub_encrypt_sha256(public_key, pout_data, &pout_len, (unsigned char *)pin_data, strlen(pin_data)) == SGX_SUCCESS) {
                s = s + "Encrypted Part 2";
        }

	size_t decrypted_len = 0;

	if(sgx_rsa_priv_decrypt_sha256(private_key, NULL, &decrypted_len, pout_data, sizeof(pout_data)) == SGX_SUCCESS) {
		s = s + "Decrypted";
	}

	unsigned char decrypted_pout_data[decrypted_len];

	if(sgx_rsa_priv_decrypt_sha256(private_key, decrypted_pout_data, &decrypted_len, pout_data, sizeof(pout_data)) == SGX_SUCCESS) {
                s = s + "Decrypted Part 2";
        }

	int c = 0;
	while(decrypted_pout_data[c] != NULL) {
		s.append(1, decrypted_pout_data[c]);
		++c;
	}

	return s;
}

//Called ONLY BY Chaincode
std::string verifyDecryptAndStoreBid(std::string user_name, std::string pin_data, shim_ctx_ptr_t ctx) {
	//Placeholder for pin_data
	
	size_t decrypted_len = 0;

	std::string s = "Test";

	sgx_rsa_result_t verify_result = SGX_RSA_INVALID_SIGNATURE;

	sgx_rsa3072_public_key_t temp_public_key;
	temp_public_key	= userSigningPublicKeys[user_name];
	unsigned char *encrypted_data;
	encrypted_data = new unsigned char [cache_data_len];


        if (sgx_rsa3072_verify(cache_signed_data, sizeof(cache_signed_data), &temp_public_key, cache_signature, &verify_result) == SGX_SUCCESS){
                s = s + "Verified signature";
        }

        if (verify_result == SGX_RSA_VALID)
        {
                s = s + "Valid Result";
		unsigned char *data = (unsigned char*)cache_signed_data;
		memcpy(encrypted_data, data, cache_data_len);
        } else {
		return "Invalid Signature Not Trusted Data";
	}
	

	if(sgx_rsa_priv_decrypt_sha256(chaincode_private_key, NULL, &decrypted_len, encrypted_data, sizeof(encrypted_data)) == SGX_SUCCESS) {
		s = s + "Decrypted";
	}

	unsigned char decrypted_pout_data[decrypted_len];

	if(sgx_rsa_priv_decrypt_sha256(chaincode_private_key, decrypted_pout_data, &decrypted_len, encrypted_data, sizeof(encrypted_data)) == SGX_SUCCESS) {
        	s = s + "Decrypted Part 2";
	}
	
	std::string auction_bid = "";
	int x = 0;
	while(decrypted_pout_data[x] != NULL) {
                auction_bid.append(1, decrypted_pout_data[x]);
                ++x;
        }
	put_state(user_name.c_str(), (uint8_t*)auction_bid.c_str(), auction_bid.size(), ctx);
	usernames[user_count] = user_name;
	user_count++;
	int c = 0;
        while(decrypted_pout_data[c] != NULL) {
                s.append(1, decrypted_pout_data[c]);
                ++c;
	}
	return s;
}

// Function to encrypt the bid for the user
// Called ONLY BY user
std::string encryptAndSign(std::string pin_data, std::string user_name, shim_ctx_ptr_t ctx) {

	char data_to_encrypt[pin_data.length()];

	int i;
	for (i = 0; i < sizeof(data_to_encrypt); i++) {
        	data_to_encrypt[i] = pin_data[i];
    	}
	size_t pout_len = 0;

	std::string s = "Test";

	if(sgx_rsa_pub_encrypt_sha256(chaincode_public_key, NULL, &pout_len, (unsigned char *)data_to_encrypt, strlen(data_to_encrypt)) == SGX_SUCCESS) {
		s = s + "Encrypted";
	}

	unsigned char pout_data[pout_len];
	
        if(sgx_rsa_pub_encrypt_sha256(chaincode_public_key, pout_data, &pout_len, (unsigned char *)data_to_encrypt, strlen(data_to_encrypt)) == SGX_SUCCESS) {
                s = s + "Encrypted Part 2";
        }

	int c = 0;
        while(pout_data[c] != NULL) {
                s.append(1, pout_data[c]);
                ++c;
        }

	sgx_rsa3072_key_t rsa_key;
	rsa_key = userSigningPrivateKeys[user_name];

	user_encrypted_data = new unsigned char [pout_len];
	memcpy(user_encrypted_data, pout_data, pout_len);
	cache_signed_data = (uint8_t*)user_encrypted_data;
	cache_data_len = pout_len;

	if (sgx_rsa3072_sign(cache_signed_data, sizeof(cache_signed_data), &rsa_key, cache_signature) == SGX_SUCCESS) {
                s = s + "Signed data";
        }
	return s;
}


//Tester function
std::string retrieveBid(std::string bid_name, shim_ctx_ptr_t ctx)
{
    	LOG_DEBUG(" +++ retrieveBid +++");
	uint32_t bid_bytes_len = -1;
	char _unencrypted_bid[128];
	const char* unencrypted_bid;

	get_state(bid_name.c_str(), (uint8_t*)_unencrypted_bid, sizeof(_unencrypted_bid) - 1, &bid_bytes_len, ctx);

	_unencrypted_bid[bid_bytes_len + 1] = '\0';
	unencrypted_bid = _unencrypted_bid;
	std::string result(unencrypted_bid);
	int length = result.length();
	result = result.substr(0, length-1);
	return result;
}

std::string storeBidMethodA(std::string user_name, std::string bid_value, shim_ctx_ptr_t ctx)
{
	
        int bid = 0;
	bool username_present = false;
	for (int i = 0; i < user_count; i++) {
		if (user_name == usernames[i]) {
			username_present = true;
			char _value[128];
        		uint32_t bid_bytes_len = -1;
        		get_state(user_name.c_str(), (uint8_t*)_value, sizeof(_value) - 1, &bid_bytes_len, ctx);
        		const char* value;
        		_value[bid_bytes_len + 1] = '\0';
        		value = _value;
        		std::string result(value);
        		int length = result.length();
                	result = result.substr(0, length-1);
                	bid = stoi(result);
		}
	}
	if (stoi(bid_value) > bid)
	{
		put_state(user_name.c_str(), (uint8_t*)bid_value.c_str(), bid_value.size(), ctx);
	}
	if (username_present == false) {
		usernames[user_count] = user_name;
		user_count++;
		return "Username added, succesful storage";
	}

	return "Succesful storage";
}

std::string retrieveAuctionResultMethodA(shim_ctx_ptr_t ctx)
{
    std::string result;
    LOG_DEBUG(" +++ retrieveBid +++");

    uint32_t bid_bytes_len = -1;
    int max = 0;
    int secondmax = 0;
    std::string username;
    std::string username_second = "";
    //Retrieve all the bids
    for (int i = 0; i < user_count; i++) {
	char _value[128];
	uint32_t bid_bytes_len = -1;	
    	get_state(usernames[i].c_str(), (uint8_t*)_value, sizeof(_value) - 1, &bid_bytes_len, ctx);
	const char* value;
	_value[bid_bytes_len + 1] = '\0';
	value = _value;
	std::string result(value);
	int length = result.length();
	result = result.substr(0, length-1);
	int bid = stoi(result);
	if (bid>max) {
		secondmax = max;
		username_second = username;
		max = bid;
		username = usernames[i];
	}
    }

    return username_second;
}

std::string asmTest(shim_ctx_ptr_t ctx)
{
	int val1,val2, add, sub, mul, finalres;
  
	std::string retString = "";
	float valx, valy, res;
	valx = 5.4;
	valy = 7.4;
   	
	int vala = (int)valx;
	int valb = (int)valy;
	val1=100;val2=20;

	asm(
        "FLDS %1 \n"
        "FLDS %2 \n"
	"movl %3, %%eax;"
	"movl %4, %%ebx;"
        "FUCOMI %%st(1), %%st \n"
	"cmovb %%ebx, %%eax;"
	"movl %%eax, %0;"
        : "=r"(finalres)
        : "m"(valy), "m"(valx), "g"(vala), "g"(valb)
        :
        );


    	if(add == 120 ){
		retString = retString + "addIsGood";
  	}
    	if (sub == 80) {
		retString = retString + "subIsGood";
    	}
    	if(mul == 2000) {
		retString = retString + "mulIsGood";
    	}
	retString = retString + std::to_string(finalres);

    	return retString;

}


std::string storeBidMethodB(std::string user_name, std::string bid_value, shim_ctx_ptr_t ctx)
{
	int bid = 0;
        bool username_present = false;
	if(user_countB < 2) {
		usernamesB[user_countB] = user_name;
                user_countB++;
	}
	for (int i = 0; i < user_countB; i++) {
		char _value[128];
                        uint32_t bid_bytes_len = -1;
                        get_state(usernamesB[i].c_str(), (uint8_t*)_value, sizeof(_value) - 1, &bid_bytes_len, ctx);
                        const char* value;
                        _value[bid_bytes_len + 1] = '\0';
                        value = _value;
                        std::string result(value);
                        int length = result.length();
                        result = result.substr(0, length-1);
                        bid = stoi(result);
	}
}

std::string retrieveAuctionResultMethodB(shim_ctx_ptr_t ctx)
{

}

//Called ONLY BY Chaincode
std::string encryptAndSignCCToUser(std::string pin_data, std::string user_name, shim_ctx_ptr_t ctx) {

	return "placeholder";
}

// Called ONLY BY User
std::string verifyDecryptAndReadResult(std::string user_name, std::string pin_data, shim_ctx_ptr_t ctx) {

	return "placeholder";
}

// implements chaincode logic for invoke
int invoke(
    uint8_t* response,
    uint32_t max_response_len,
    uint32_t* actual_response_len,
    shim_ctx_ptr_t ctx)
{
    //LOG_DEBUG("Harsh: +++ Executing chaincode invocation +++");

    std::string function_name;
    std::vector<std::string> params;
    get_func_and_params(function_name, params, ctx);
    std::string result;

    if (function_name == "putTestVariable")
    {
	    std::string user_name = params[0];
	    result = putTestVariable(user_name, ctx);
    } 
    else if (function_name == "getTestVariable")
    {
            result = getTestVariable(ctx);
    }
    else if (function_name == "createUserPublicPrivateKey")
    {
	    std::string user_name = params[0];
	    result = createUserPublicPrivateKey(user_name, ctx);
    }
    else if (function_name == "createChaincodePublicPrivateKey")
    {
		result = createChaincodePublicPrivateKey(ctx);
    }
    else if (function_name == "retrieveChaincodePublicKey")
    {
	result = retrieveChaincodePublicKey(ctx);
    }
    else if (function_name == "encryptionSimulation")
    {
	result = encryptionSimulation(ctx);    
    }
    else if (function_name == "verifyDecryptAndStoreBid")
    {
	std::string value = params[1]; 
        std::string user_name = params[0];
        result = verifyDecryptAndStoreBid(user_name, value, ctx);
    }
    else if (function_name == "asmTest")
    {
	result = asmTest(ctx);
    }
    else if (function_name == "encryptAndSign")
    {
	std::string data = params[0];
	std::string user_name = params[1];
	result = encryptAndSign( data, user_name, ctx);
    }
    else if (function_name == "retrieveBid")
    {
	std::string user_name = params[0];
	result = retrieveBid(user_name, ctx);
    }
    else if (function_name == "storeBidMethodA")
    {
	std::string user_name = params[0];
	std::string value = params[1];
	result = storeBidMethodA(user_name, value, ctx);
    }
    else if (function_name == "storeBidMethodB")
    {
	std::string user_name = params[0];
	std::string value = params[1];
	result = storeBidMethodB(user_name, value, ctx);
    }
    else if (function_name == "retrieveAuctionResultMethodA")
    {
        result = retrieveAuctionResultMethodA(ctx);
    }
    else if (function_name == "retrieveAuctionResultMethodB")
    {
        result = retrieveAuctionResultMethodB(ctx);
    }
    else if (function_name == "signingSimulation")
    {
	result = signingSimulation(ctx);
    }
    else if (function_name == "encryptAndSignCCToUser")
    {
	std::string data = params[0];
        std::string user_name = params[1];
        result = encryptAndSignCCToUser( data, user_name, ctx);
    }
    else if (function_name == "verifyDecryptAndReadResult")
    {
	std::string value = params[1];
        std::string user_name = params[0];
        result = verifyDecryptAndReadResult(user_name, value, ctx);
    }
    else
    {
        // unknown function
        LOG_DEBUG("Harsh: RECEIVED UNKNOWN transaction '%s'", function_name);
        return -1;
    }

    // check that result fits into response
    int neededSize = result.size();
    if (max_response_len < neededSize)
    {
        // error:  buffer too small for the response to be sent
        *actual_response_len = 0;
        return -1;
    }

    // copy result to response
    memcpy(response, result.c_str(), neededSize);
    *actual_response_len = neededSize;
    LOG_DEBUG("Harsh: Response: %s", result.c_str());
    LOG_DEBUG("+++ Executing done +++");
    return 0;
}
