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

std::map<int, std::pair<std::string, int>> bids;
#define OK "OK"
#define NOT_FOUND "Bid not found"
#define RSA_MOD_SIZE 384 //hardcode n size to be 384
#define RSA_E_SIZE 4 //hardcode e size to be 4

#define MAX_VALUE_SIZE 1024
int user_count = 0;
std::map<int, std::string> usernames;

std::map<int, std::pair<std::string, int>> record;

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

// Initializor function

std::string clearRecord(shim_ctx_ptr_t ctx)
{
	std::pair<std::string, int> BIDDER1, BIDDER2;
        BIDDER1.first = "nobody";
        BIDDER1.second = 0;
	BIDDER2.first = "nobody2";
        BIDDER2.second = 0;
        record[0] = BIDDER1;
	record[1] = BIDDER2;

	return "initialization complete";
		
}

// Reset the auction
std::string resetAuction(shim_ctx_ptr_t ctx)
{
	std::pair<std::string, int> BIDDER;
        BIDDER.first = "nobody";
        BIDDER.second = 0;
	for (int i = 0; i < user_count; i++) {
		bids[i] = BIDDER;
	}
	user_count = 0;
	return "auction is reset";
}

// Shower function
std::string showRecord(shim_ctx_ptr_t ctx)
{
	std::string returnstatus = "";
	std::pair<std::string, int> BIDDER1, BIDDER2;
	BIDDER1 = record[0];
        BIDDER2 = record[1];

	returnstatus = returnstatus + BIDDER1.first;
	returnstatus = returnstatus + std::to_string(BIDDER1.second);
	returnstatus = returnstatus + BIDDER2.first;
	returnstatus = returnstatus + std::to_string(BIDDER2.second);

	return returnstatus;
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

std::map<int, std::pair<std::string, int>> oblivious(int value, std::string bidder)
{
	std::pair<std::string, int> BIDDER, BIDDER1, BIDDER2;
	BIDDER.first = bidder;
        BIDDER.second = value;

	BIDDER1 = record[0];
	BIDDER2 = record[1];

	int vala, valb;
	int index0 = 0;
        int index1 = 1;
	int index2 = 2;

	std::map <int, std::string> bidderlist;
	bidderlist[0] = BIDDER1.first;
	bidderlist[1] = BIDDER2.first;
	bidderlist[2] = bidder;


	// Replace by asm
	if(value > BIDDER1.second) {
		record[1] = record[0];
		record[0] = BIDDER;
	} else if((value < BIDDER1.second) && (value > BIDDER2.second)) {
		record[1] = BIDDER;
	}
	//The replacement
	//asm(
        //"FLDS %1 \n"
        //"FLDS %2 \n"
        //"movl %3, %%eax;"
        //"movl %4, %%ebx;"
        //"FUCOMI %%st(1), %%st \n"
        //"cmovb %%eax, %%ebx;"
        //"movl %%ebx, %0;"
        //"clc;"
        //: "=r"(finalresult)
        //: "m"(valx), "m"(valy), "g"(vala), "g"(valb)
        //:
        //);

	BIDDER1.first = bidderlist[index0];
	BIDDER1.second = vala;

	BIDDER2.first = bidderlist[index1];
	BIDDER2.second = valb;

	record[0] = BIDDER1;
	record[1] = BIDDER2;

	return record;

}


std::string storeBidMethodA(std::string user_name, std::string bid_value, shim_ctx_ptr_t ctx)
{
	std::string returnStatusString = "";
	std::pair<std::string, int> BIDDER;
        BIDDER.first = user_name;
        BIDDER.second = stoi(bid_value);
	bids[user_count] = BIDDER;
	put_state(user_name.c_str(), (uint8_t*)bid_value.c_str(), bid_value.size(), ctx);
	usernames[user_count] = user_name;
	user_count = user_count + 1;
	return returnStatusString;
}

std::string retrieveAuctionResultMethodA(shim_ctx_ptr_t ctx)
{
    std::string bidString;
    std::string returnstatus = "";
    std::map<int, std::pair<std::string, int>> finalresult;
    std::pair<std::string, int> BIDDER;
    std::string username;
    //Retrieve all the bids
    for (int i = 0; i < user_count; i++) {
	char _value[128];
	uint32_t bid_bytes_len = -1;	
    	get_state(usernames[i].c_str(), (uint8_t*)_value, sizeof(_value) - 1, &bid_bytes_len, ctx);
	const char* value;
	_value[bid_bytes_len + 1] = '\0';
	value = _value;
	std::string bidString(value);
	int length = bidString.length();
	bidString = bidString.substr(0, length-1);
	int bid = stoi(bidString);
	BIDDER = bids[i];
	bid = BIDDER.second;
	username = BIDDER.first;

	// Obliviously retrieve the maximum
	finalresult = oblivious(bid, username);

    }

    std::pair<std::string, int> BIDDER1, BIDDER2;
    BIDDER1 = finalresult[0];
    BIDDER2 = finalresult[1];


    returnstatus = returnstatus + " The winner is " + BIDDER1.first;
    returnstatus = returnstatus + " And had originally bid " + std::to_string(BIDDER1.second);
    returnstatus = returnstatus + " But pays the second price of " +std::to_string(BIDDER2.second);
    returnstatus = returnstatus + " That was bid by " + BIDDER2.first;
    return returnstatus;


}


std::string storeBidMethodB(std::string user_name, std::string bid_value, shim_ctx_ptr_t ctx)
{

	std::string returnStatusString = "";
        std::map<int, std::pair<std::string, int>> finalresult;
	std::pair<std::string, int> BIDDER;
        BIDDER.first = user_name;
        BIDDER.second = stoi(bid_value);
        bids[user_count] = BIDDER;
        put_state(user_name.c_str(), (uint8_t*)bid_value.c_str(), bid_value.size(), ctx);
        usernames[user_count] = user_name;
        user_count = user_count + 1;
	finalresult = oblivious(BIDDER.second, BIDDER.first);
	return returnStatusString;
}

std::string retrieveAuctionResultMethodB(shim_ctx_ptr_t ctx)
{
	std::string returnstatus = "";
	std::pair<std::string, int> BIDDER1, BIDDER2;
    	BIDDER1 = record[0];
    	BIDDER2 = record[1];

    	returnstatus = returnstatus + " The winner is " + BIDDER1.first;
    	returnstatus = returnstatus + " And had originally bid " + std::to_string(BIDDER1.second);
    	returnstatus = returnstatus + " But pays the second price of " +std::to_string(BIDDER2.second);
    	returnstatus = returnstatus + " That was bid by " + BIDDER2.first;

	return returnstatus;
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
    else if (function_name == "clearRecord")
    {
	result = clearRecord(ctx);
    }
    else if (function_name == "resetAuction")
    {
        result = resetAuction(ctx);
    }
    else if (function_name == "showRecord")
    {
	result = showRecord(ctx);
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
