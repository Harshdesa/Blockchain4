start=`date +%s.%N`
if [ $# -eq 0 ]; then
	echo "What is the chaincode id?"
	exit 1
fi

docker exec -e CORE_PEER_LOCALMSPID=Org1MSP -e CORE_PEER_MSPCONFIGPATH=/etc/hyperledger/msp/users/Admin@org1.example.com/msp peer0.org1.example.com env TERM=${TERM} /project/src/github.com/hyperledger-labs/fabric-private-chaincode/fabric/bin/peer.sh chaincode invoke -o orderer.example.com:7050 -C mychannel -n $1 -c '{"Args":["clearRecord"]}' --waitForEvent

docker exec -e CORE_PEER_LOCALMSPID=Org1MSP -e CORE_PEER_MSPCONFIGPATH=/etc/hyperledger/msp/users/Admin@org1.example.com/msp peer0.org1.example.com env TERM=${TERM} /project/src/github.com/hyperledger-labs/fabric-private-chaincode/fabric/bin/peer.sh chaincode invoke -o orderer.example.com:7050 -C mychannel -n $1 -c '{"Args":["storeBid","newyork", "5"]}' --waitForEvent

docker exec -e CORE_PEER_LOCALMSPID=Org1MSP -e CORE_PEER_MSPCONFIGPATH=/etc/hyperledger/msp/users/Admin@org1.example.com/msp peer0.org1.example.com env TERM=${TERM} /project/src/github.com/hyperledger-labs/fabric-private-chaincode/fabric/bin/peer.sh chaincode invoke -o orderer.example.com:7050 -C mychannel -n $1 -c '{"Args":["storeBid","losangeles", "10"]}' --waitForEvent

docker exec -e CORE_PEER_LOCALMSPID=Org1MSP -e CORE_PEER_MSPCONFIGPATH=/etc/hyperledger/msp/users/Admin@org1.example.com/msp peer0.org1.example.com env TERM=${TERM} /project/src/github.com/hyperledger-labs/fabric-private-chaincode/fabric/bin/peer.sh chaincode invoke -o orderer.example.com:7050 -C mychannel -n $1 -c '{"Args":["storeBid","chicago", "15"]}' --waitForEvent

docker exec -e CORE_PEER_LOCALMSPID=Org1MSP -e CORE_PEER_MSPCONFIGPATH=/etc/hyperledger/msp/users/Admin@org1.example.com/msp peer0.org1.example.com env TERM=${TERM} /project/src/github.com/hyperledger-labs/fabric-private-chaincode/fabric/bin/peer.sh chaincode invoke -o orderer.example.com:7050 -C mychannel -n $1 -c '{"Args":["storeBid","huoston", "20"]}' --waitForEvent

docker exec -e CORE_PEER_LOCALMSPID=Org1MSP -e CORE_PEER_MSPCONFIGPATH=/etc/hyperledger/msp/users/Admin@org1.example.com/msp peer0.org1.example.com env TERM=${TERM} /project/src/github.com/hyperledger-labs/fabric-private-chaincode/fabric/bin/peer.sh chaincode invoke -o orderer.example.com:7050 -C mychannel -n $1 -c '{"Args":["storeBid","dallas", "30"]}' --waitForEvent

docker exec -e CORE_PEER_LOCALMSPID=Org1MSP -e CORE_PEER_MSPCONFIGPATH=/etc/hyperledger/msp/users/Admin@org1.example.com/msp peer0.org1.example.com env TERM=${TERM} /project/src/github.com/hyperledger-labs/fabric-private-chaincode/fabric/bin/peer.sh chaincode invoke -o orderer.example.com:7050 -C mychannel -n $1 -c '{"Args":["storeBid","miami", "45"]}' --waitForEvent

docker exec -e CORE_PEER_LOCALMSPID=Org1MSP -e CORE_PEER_MSPCONFIGPATH=/etc/hyperledger/msp/users/Admin@org1.example.com/msp peer0.org1.example.com env TERM=${TERM} /project/src/github.com/hyperledger-labs/fabric-private-chaincode/fabric/bin/peer.sh chaincode invoke -o orderer.example.com:7050 -C mychannel -n $1 -c '{"Args":["storeBid","austin", "40"]}' --waitForEvent


docker exec -e CORE_PEER_LOCALMSPID=Org1MSP -e CORE_PEER_MSPCONFIGPATH=/etc/hyperledger/msp/users/Admin@org1.example.com/msp peer0.org1.example.com env TERM=${TERM} /project/src/github.com/hyperledger-labs/fabric-private-chaincode/fabric/bin/peer.sh chaincode invoke -o orderer.example.com:7050 -C mychannel -n $1 -c '{"Args":["storeBid","newyorka", "50"]}' --waitForEvent

docker exec -e CORE_PEER_LOCALMSPID=Org1MSP -e CORE_PEER_MSPCONFIGPATH=/etc/hyperledger/msp/users/Admin@org1.example.com/msp peer0.org1.example.com env TERM=${TERM} /project/src/github.com/hyperledger-labs/fabric-private-chaincode/fabric/bin/peer.sh chaincode invoke -o orderer.example.com:7050 -C mychannel -n $1 -c '{"Args":["storeBid","losangelesa", "100"]}' --waitForEvent

docker exec -e CORE_PEER_LOCALMSPID=Org1MSP -e CORE_PEER_MSPCONFIGPATH=/etc/hyperledger/msp/users/Admin@org1.example.com/msp peer0.org1.example.com env TERM=${TERM} /project/src/github.com/hyperledger-labs/fabric-private-chaincode/fabric/bin/peer.sh chaincode invoke -o orderer.example.com:7050 -C mychannel -n $1 -c '{"Args":["storeBid","chicagoa", "150"]}' --waitForEvent

docker exec -e CORE_PEER_LOCALMSPID=Org1MSP -e CORE_PEER_MSPCONFIGPATH=/etc/hyperledger/msp/users/Admin@org1.example.com/msp peer0.org1.example.com env TERM=${TERM} /project/src/github.com/hyperledger-labs/fabric-private-chaincode/fabric/bin/peer.sh chaincode invoke -o orderer.example.com:7050 -C mychannel -n $1 -c '{"Args":["storeBid","huostona", "200"]}' --waitForEvent

docker exec -e CORE_PEER_LOCALMSPID=Org1MSP -e CORE_PEER_MSPCONFIGPATH=/etc/hyperledger/msp/users/Admin@org1.example.com/msp peer0.org1.example.com env TERM=${TERM} /project/src/github.com/hyperledger-labs/fabric-private-chaincode/fabric/bin/peer.sh chaincode invoke -o orderer.example.com:7050 -C mychannel -n $1 -c '{"Args":["storeBid","dallasa", "300"]}' --waitForEvent

docker exec -e CORE_PEER_LOCALMSPID=Org1MSP -e CORE_PEER_MSPCONFIGPATH=/etc/hyperledger/msp/users/Admin@org1.example.com/msp peer0.org1.example.com env TERM=${TERM} /project/src/github.com/hyperledger-labs/fabric-private-chaincode/fabric/bin/peer.sh chaincode invoke -o orderer.example.com:7050 -C mychannel -n $1 -c '{"Args":["storeBid","miamia", "450"]}' --waitForEvent

docker exec -e CORE_PEER_LOCALMSPID=Org1MSP -e CORE_PEER_MSPCONFIGPATH=/etc/hyperledger/msp/users/Admin@org1.example.com/msp peer0.org1.example.com env TERM=${TERM} /project/src/github.com/hyperledger-labs/fabric-private-chaincode/fabric/bin/peer.sh chaincode invoke -o orderer.example.com:7050 -C mychannel -n $1 -c '{"Args":["storeBid","austina", "400"]}' --waitForEvent

end=`date +%s.%N`
runtime=$( echo "$end - $start" | bc -l )

echo "Runtime to upload bids is: "
echo $runtime

start=`date +%s.%N`
docker exec -e CORE_PEER_LOCALMSPID=Org1MSP -e CORE_PEER_MSPCONFIGPATH=/etc/hyperledger/msp/users/Admin@org1.example.com/msp peer0.org1.example.com env TERM=${TERM} /project/src/github.com/hyperledger-labs/fabric-private-chaincode/fabric/bin/peer.sh chaincode invoke -o orderer.example.com:7050 -C mychannel -n $1 -c '{"Args":["retrieveAuctionResultFirstPrice"]}' --waitForEvent
end=`date +%s.%N`
runtime=$( echo "$end - $start" | bc -l )

echo "Runtime to calculate first price auction is: "
echo $runtime

start=`date +%s.%N`
docker exec -e CORE_PEER_LOCALMSPID=Org1MSP -e CORE_PEER_MSPCONFIGPATH=/etc/hyperledger/msp/users/Admin@org1.example.com/msp peer0.org1.example.com env TERM=${TERM} /project/src/github.com/hyperledger-labs/fabric-private-chaincode/fabric/bin/peer.sh chaincode invoke -o orderer.example.com:7050 -C mychannel -n $1 -c '{"Args":["retrieveAuctionResultSecondPrice"]}' --waitForEvent
end=`date +%s.%N`
runtime=$( echo "$end - $start" | bc -l )

echo "Runtime to calculate second price auction is: "
echo $runtime


docker exec -e CORE_PEER_LOCALMSPID=Org1MSP -e CORE_PEER_MSPCONFIGPATH=/etc/hyperledger/msp/users/Admin@org1.example.com/msp peer0.org1.example.com env TERM=${TERM} /project/src/github.com/hyperledger-labs/fabric-private-chaincode/fabric/bin/peer.sh chaincode invoke -o orderer.example.com:7050 -C mychannel -n $1 -c '{"Args":["resetAuction"]}' --waitForEvent
#
#docker exec -e CORE_PEER_LOCALMSPID=Org1MSP -e CORE_PEER_MSPCONFIGPATH=/etc/hyperledger/msp/users/Admin@org1.example.com/msp peer0.org1.example.com env TERM=${TERM} /project/src/github.com/hyperledger-labs/fabric-private-chaincode/fabric/bin/peer.sh chaincode invoke -o orderer.example.com:7050 -C mychannel -n $1 -c '{"Args":["clearRecord"]}' --waitForEvent
#
#docker exec -e CORE_PEER_LOCALMSPID=Org1MSP -e CORE_PEER_MSPCONFIGPATH=/etc/hyperledger/msp/users/Admin@org1.example.com/msp peer0.org1.example.com env TERM=${TERM} /project/src/github.com/hyperledger-labs/fabric-private-chaincode/fabric/bin/peer.sh chaincode invoke -o orderer.example.com:7050 -C mychannel -n $1 -c '{"Args":["storeBidMethodB","newyork", "5"]}' --waitForEvent
#
#docker exec -e CORE_PEER_LOCALMSPID=Org1MSP -e CORE_PEER_MSPCONFIGPATH=/etc/hyperledger/msp/users/Admin@org1.example.com/msp peer0.org1.example.com env TERM=${TERM} /project/src/github.com/hyperledger-labs/fabric-private-chaincode/fabric/bin/peer.sh chaincode invoke -o orderer.example.com:7050 -C mychannel -n $1 -c '{"Args":["storeBidMethodB","losangeles", "10"]}' --waitForEvent
#
#docker exec -e CORE_PEER_LOCALMSPID=Org1MSP -e CORE_PEER_MSPCONFIGPATH=/etc/hyperledger/msp/users/Admin@org1.example.com/msp peer0.org1.example.com env TERM=${TERM} /project/src/github.com/hyperledger-labs/fabric-private-chaincode/fabric/bin/peer.sh chaincode invoke -o orderer.example.com:7050 -C mychannel -n $1 -c '{"Args":["storeBidMethodB","chicago", "15"]}' --waitForEvent
#
#docker exec -e CORE_PEER_LOCALMSPID=Org1MSP -e CORE_PEER_MSPCONFIGPATH=/etc/hyperledger/msp/users/Admin@org1.example.com/msp peer0.org1.example.com env TERM=${TERM} /project/src/github.com/hyperledger-labs/fabric-private-chaincode/fabric/bin/peer.sh chaincode invoke -o orderer.example.com:7050 -C mychannel -n $1 -c '{"Args":["storeBidMethodB","huoston", "20"]}' --waitForEvent
#
#docker exec -e CORE_PEER_LOCALMSPID=Org1MSP -e CORE_PEER_MSPCONFIGPATH=/etc/hyperledger/msp/users/Admin@org1.example.com/msp peer0.org1.example.com env TERM=${TERM} /project/src/github.com/hyperledger-labs/fabric-private-chaincode/fabric/bin/peer.sh chaincode invoke -o orderer.example.com:7050 -C mychannel -n $1 -c '{"Args":["storeBidMethodB","losangeles", "30"]}' --waitForEvent
#
#docker exec -e CORE_PEER_LOCALMSPID=Org1MSP -e CORE_PEER_MSPCONFIGPATH=/etc/hyperledger/msp/users/Admin@org1.example.com/msp peer0.org1.example.com env TERM=${TERM} /project/src/github.com/hyperledger-labs/fabric-private-chaincode/fabric/bin/peer.sh chaincode invoke -o orderer.example.com:7050 -C mychannel -n $1 -c '{"Args":["storeBidMethodB","newyork", "45"]}' --waitForEvent
#
#docker exec -e CORE_PEER_LOCALMSPID=Org1MSP -e CORE_PEER_MSPCONFIGPATH=/etc/hyperledger/msp/users/Admin@org1.example.com/msp peer0.org1.example.com env TERM=${TERM} /project/src/github.com/hyperledger-labs/fabric-private-chaincode/fabric/bin/peer.sh chaincode invoke -o orderer.example.com:7050 -C mychannel -n $1 -c '{"Args":["storeBidMethodB","huoston", "40"]}' --waitForEvent
#
#docker exec -e CORE_PEER_LOCALMSPID=Org1MSP -e CORE_PEER_MSPCONFIGPATH=/etc/hyperledger/msp/users/Admin@org1.example.com/msp peer0.org1.example.com env TERM=${TERM} /project/src/github.com/hyperledger-labs/fabric-private-chaincode/fabric/bin/peer.sh chaincode invoke -o orderer.example.com:7050 -C mychannel -n $1 -c '{"Args":["retrieveAuctionResultMethodB"]}' --waitForEvent
