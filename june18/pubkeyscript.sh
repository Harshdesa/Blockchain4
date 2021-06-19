if [ $# -eq 2 ]; then
	echo "What is the chaincode id?"
	exit 1
fi



docker exec -e CORE_PEER_LOCALMSPID=Org1MSP -e CORE_PEER_MSPCONFIGPATH=/etc/hyperledger/msp/users/Admin@org1.example.com/msp peer0.org1.example.com env TERM=${TERM} /project/src/github.com/hyperledger-labs/fabric-private-chaincode/fabric/bin/peer.sh chaincode invoke -o orderer.example.com:7050 -C mychannel -n $1 -c '{"Args":["createKeys"]}' --waitForEvent 2> publickey
echo "this is the public key"
cat publickey | grep -oP '(?<=ResponseData\\\"\:\\\").*' | cut -c 1-836

echo "this is the encrypted text"
docker exec -e CORE_PEER_LOCALMSPID=Org1MSP -e CORE_PEER_MSPCONFIGPATH=/etc/hyperledger/msp/users/Admin@org1.example.com/msp peer0.org1.example.com env TERM=${TERM} /project/src/github.com/hyperledger-labs/fabric-private-chaincode/fabric/bin/peer.sh chaincode invoke -o orderer.example.com:7050 -C mychannel -n $1 -c '{"Args":["encryptMessage", "hellowrold"]}' 2> encryptedtextfile

cat encryptedtextfile | grep -oP '(?<=ResponseData\\\"\:\\\").*' | cut -c 1-512


#docker exec -e CORE_PEER_LOCALMSPID=Org1MSP -e CORE_PEER_MSPCONFIGPATH=/etc/hyperledger/msp/users/Admin@org1.example.com/msp peer0.org1.example.com env TERM=${TERM} /project/src/github.com/hyperledger-labs/fabric-private-chaincode/fabric/bin/peer.sh chaincode invoke -o orderer.example.com:7050 -C mychannel -n $1 -c '{"Args":["testRsaEncryption", "kFlfJGJ/AAA=", "8GhfJGJ/AAA="]}' --waitForEvent
