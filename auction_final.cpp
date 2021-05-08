using namespace std;
#include "shim.h"
#include <map>
#include <sstream>
#include "logging.h"
#include <string>
#include <ctime>
#include <cstring>
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

std::map<std::string, int> bids;
std::map<int, std::string> bidder;
std::map<std::string, int> bidsb;
std::map<std::string, int> bidderb;
#define OK "OK"
#define NOT_FOUND "Bid not found"
#define RSA_MOD_SIZE 384 //hardcode n size to be 384
#define RSA_E_SIZE 4 //hardcode e size to be 4

#define MAX_VALUE_SIZE 1024
int user_count = 0;
std::map<int, std::string> usernames;
std::map<int, std::string> usernamesB;
int user_countB = 2;
int maximum_methodB = 0;
int secondmaximum_methodB = 0;



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

//Tester function
std::string retrieveUsernames(shim_ctx_ptr_t ctx) {
	
	std::string returnUsernames = "";
	for (int i = 0; i < user_count; i++) {
		returnUsernames = returnUsernames + usernames[i];
	}
	return returnUsernames;

}
//Tester function
std::string asmTest(shim_ctx_ptr_t ctx)
{
	int finalres;
  
	std::string retString = "";
	float valx, valy;
	valx = 3.0;
	valy = 25.0;
   	
	int vala = (int)valx;
	int valb = (int)valy;

	asm(
        "FLDS %1 \n"
        "FLDS %2 \n"
	"movl %3, %%eax;"
	"movl %4, %%ebx;"
        "FUCOMI %%st(1), %%st \n"
	"cmovb %%eax, %%ebx;"
	"movl %%ebx, %0;"
	"clc;"
        : "=r"(finalres)
        : "m"(valx), "m"(valy), "g"(vala), "g"(valb)
        :
        );

	retString = retString + std::to_string(finalres);

    	return retString;

}

int obliviousMax(int vala, int valb)
{
	int finalresult;
	float valx = (float) vala;
	float valy = (float) valb;

	asm(
        "FLDS %1 \n"
        "FLDS %2 \n"
        "movl %3, %%eax;"
        "movl %4, %%ebx;"
        "FUCOMI %%st(1), %%st \n"
        "cmovb %%eax, %%ebx;"
        "movl %%ebx, %0;"
	"clc;"
        : "=r"(finalresult)
        : "m"(valx), "m"(valy), "g"(vala), "g"(valb)
        :
        );

	return finalresult;
}

bool checkEqual(int vala, int valb)
{
	if(vala == valb)
	{
		return true;
	} else
		return false;
}

std::string storeBidMethodA(std::string user_name, std::string bid_value, shim_ctx_ptr_t ctx)
{
	std::string returnStatusString = "";
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
			returnStatusString = returnStatusString + "Bidder exists, existing Bid read from the blockchain";
		}
	}
	if (stoi(bid_value) > bid)
	{
		bids[user_name] = stoi(bid_value);
		bidder[stoi(bid_value)] = user_name;
		put_state(user_name.c_str(), (uint8_t*)bid_value.c_str(), bid_value.size(), ctx);
		returnStatusString = returnStatusString + "Blockchain State Updated";
	}
	if (username_present == false) {
		usernames[user_count] = user_name;
		user_count = user_count + 1;
		returnStatusString = returnStatusString + "Bidder added, successful storage";
	}
	return returnStatusString;
}

std::string retrieveAuctionResultMethodA(shim_ctx_ptr_t ctx)
{
    std::string result;

    int max = 0;
    int secondmax = 0;
    std::string username;
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
	bid = bids[usernames[i]];

	// Obliviously retrieve the maximum
	max = obliviousMax(bid, max);

    }

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
	bid = bids[usernames[i]];

	//Obliviously exclude the maximum
	if(!checkEqual(bid,max)) {
		secondmax = obliviousMax(bid, secondmax);
	}
    }

    return bidder[secondmax];


}


std::string storeBidMethodB(std::string user_name, std::string bid_value, shim_ctx_ptr_t ctx)
{
	int bid = stoi(bid_value);
	int secondmax = obliviousMax(bid, secondmaximum_methodB);
	int max = obliviousMax(bid, maximum_methodB);
	
	
	if(checkEqual(max, bid)) {
		usernamesB[1] = usernamesB[0];
		usernamesB[0] = user_name;
		bidsb[user_name] = bid;
		put_state(user_name.c_str(), (uint8_t*)bid_value.c_str(), bid_value.size(), ctx);
		secondmaximum_methodB = maximum_methodB;
		maximum_methodB = bid;
	} else if (checkEqual(secondmax, bid)) {
		usernamesB[1] = user_name;
		bidsb[user_name] = bid;
		put_state(user_name.c_str(), (uint8_t*)bid_value.c_str(), bid_value.size(), ctx);
		secondmaximum_methodB = bid;
	}

}

std::string retrieveAuctionResultMethodB(shim_ctx_ptr_t ctx)
{
	return usernamesB[1];
}


// implements chaincode logic for invoke
int invoke(
    uint8_t* response,
    uint32_t max_response_len,
    uint32_t* actual_response_len,
    shim_ctx_ptr_t ctx)
{

    std::string function_name;
    std::vector<std::string> params;
    get_func_and_params(function_name, params, ctx);
    std::string result;

    if (function_name == "asmTest")
    {
	result = asmTest(ctx);
    }
    else if (function_name == "retrieveBid")
    {
	std::string user_name = params[0];
	result = retrieveBid(user_name, ctx);
    }
    else if (function_name == "retrieveUsernames")
    {
	result = retrieveUsernames(ctx);
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
