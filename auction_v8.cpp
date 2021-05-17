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
	std::pair<std::string, int> BIDDERMAX, BIDDERSECONDMAX;
        BIDDERMAX.first = "nobody";
        BIDDERMAX.second = 0;
	BIDDERSECONDMAX.first = "nobody2";
        BIDDERSECONDMAX.second = 0;
        record[0] = BIDDERMAX;
	record[1] = BIDDERSECONDMAX;

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
	std::pair<std::string, int> BIDDERMAX, BIDDERSECONDMAX;
	BIDDERMAX = record[0];
        BIDDERSECONDMAX = record[1];

	returnstatus = returnstatus + BIDDERMAX.first;
	returnstatus = returnstatus + std::to_string(BIDDERMAX.second);
	returnstatus = returnstatus + BIDDERSECONDMAX.first;
	returnstatus = returnstatus + std::to_string(BIDDERSECONDMAX.second);

	return returnstatus;
}


std::map<int, std::pair<std::string, int>> oblivious(int value, std::string bidder)
{
	std::pair<std::string, int> BIDDER, BIDDERMAX, BIDDERSECONDMAX;
	BIDDER.first = bidder;
        BIDDER.second = value;

	BIDDERMAX = record[0];
	BIDDERSECONDMAX = record[1];

	int max, secondmax;
	int biddermaxid=1;

	std::map <int, std::string> bidderlist;
	bidderlist[BIDDERMAX.second] = BIDDERMAX.first;
	bidderlist[BIDDERSECONDMAX.second] = BIDDERSECONDMAX.first;
	bidderlist[value] = bidder;


	/* Replace by asm
	* if(value > BIDDERMAX.second) {
	*	record[1] = record[0];
	*	record[0] = BIDDER;
	* } else if((value < BIDDERMAX.second) && (value > BIDDERSECONDMAX.second)) {
	*	record[1] = BIDDER;
	*}
	*/
	
	
	//The replacement
	// eax = value to test
	// ebx = global maximum
	// compare eax and ebx, return max if carry flag is set
	// finally ecx will hold either a 0 or 1 value identifying the max bidder

	asm(
	"clc \n"
	"movl %%eax, %1 \n"      
	"movl %%ebx, %2 \n"      
	"cmp %%ebx, %%eax \n"    
	"cmovb %%ebx, %%eax \n"  
	"movl %0, %%ebx \n"
	: "=r"(max)
	: "a"(value), "b"(BIDDERMAX.second)
	);

	// eax = maximum from previous asm block
	// ebx = global maximum
	// ecx = global second maximum
	// edx = value to test
	// compare eax and ebx, if not equal then secondmax(ecx) = global maximum(ebx)
	// But, what if eax and ebx are equal? This means that value could be greater than global second maximum. 
	// Therefore, compare edx and ecx, if edx > ecx, make secondmax = value    ... (1)

	asm(
	"clc \n"
	"movl %1, %%eax \n"
	"movl %2, %%ebx \n"
	"movl %3, %%ecx \n"
	"movl %4, %%edx \n"
	"cmp %%eax, %%ebx \n"
	"cmovne %%ebx, %%ecx \n"
	"clc \n"
	"cmp %%edx, %%ecx \n"
	"cmovb %%edx, %%ecx \n"
	"movl %%ecx, %0 \n"
	: "=r"(secondmax)
	: "g"(max), "g"(BIDDERMAX.second), "g"(BIDDERSECONDMAX.second), "g"(value)
	);


	// This is an edge case due to the last comparison in the previous asm block (1).
	// The last comparsion in the above asm block ALWAYS happens. So, if value > max, then maximum becomes value. But, due to the previous comparison, second max also becomes value
	// Therefore, here we compare max and the secondmax. If both are equal, then secondmax will be the global maximum value. 

	asm(
	"clc \n"
	"movl %1, %%eax \n"
	"movl %2, %%ebx \n"
	"movl %3, %%ecx \n"
	"cmp %%eax, %%ebx \n"
	"cmove %%ecx, %%ebx \n"
	"movl %%ebx, %0 \n"
	: "=r"(secondmax)
	:"g"(max), "g"(secondmax), "g"(BIDDERMAX.second)
	);
	

	BIDDERMAX.second = max;
	BIDDERMAX.first = bidderlist[BIDDERMAX.second];

	BIDDERSECONDMAX.first = "";
	BIDDERSECONDMAX.second = secondmax;

	record[0] = BIDDERMAX;
	record[1] = BIDDERSECONDMAX;

	return record;

}

int maximum(int bid1, int bid2) {

	int returnvalue = 0;
	
	asm(
	"clc \n"
	"movl %1, %%eax \n"
	"movl %2, %%ebx \n"
	"movl $0, %%ecx \n"
	"movl $0, %%edx \n"
	"cmp %%ebx, %%eax \n"
	"adc %%ecx, %%edx \n"
	"movl %0, %%edx"
	: "=d"(returnvalue)
	: "g"(bid1), "g"(bid2)
	);

	return returnvalue;

	//if (bid1 > bid2) {
	//	return 0;
	//} else
	//{
	//	return 1;
	//}
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
    int smax = 0;
    int max = 0;
    int maxid = 0;
    int smaxid = 0;
    int b = 0;
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
	//finalresult = oblivious(bid, username);
	b = maximum(smax, bid);
	smax = (1-b)*smax + b*bid;
	smaxid = (1-b)*smaxid + b*i;

	b = maximum(max, smax);
	int tempmax = max;
	int tempmaxid = maxid;
	max = (1-b)*max + b*smax;
	maxid = (1-b)*maxid + b*smaxid;

	smax = b*tempmax + (1-b)*smax;
	smaxid = b*tempmaxid + (1-b)*smaxid;


    }

    std::pair<std::string, int> BIDDERMAX, BIDDERSECONDMAX;
    BIDDERMAX = finalresult[0];
    BIDDERSECONDMAX = finalresult[1];


    returnstatus = returnstatus + " The winner is " + usernames[maxid];
    returnstatus = returnstatus + " And had originally bid " + std::to_string(max);
    returnstatus = returnstatus + " But pays the second price of " + usernames[smaxid];
    returnstatus = returnstatus + " That was bid by " + std::to_string(smax);
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
	std::pair<std::string, int> BIDDERMAX, BIDDERSECONDMAX;
    	BIDDERMAX = record[0];
    	BIDDERSECONDMAX = record[1];

    	returnstatus = returnstatus + " The winner is " + BIDDERMAX.first;
    	returnstatus = returnstatus + " And had originally bid " + std::to_string(BIDDERMAX.second);
    	returnstatus = returnstatus + " But pays the second price of " +std::to_string(BIDDERSECONDMAX.second);
    	returnstatus = returnstatus + " That was bid by " + BIDDERSECONDMAX.first;

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
