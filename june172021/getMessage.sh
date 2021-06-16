
echo "$1" | base64 --decode > raw

openssl rsautl -decrypt -inkey keys/private.pem -in raw -out normal.txt

VAR=$(cat normal.txt)

docker exec -e CORE_PEER_LOCALMSPID=Org1MSP -e CORE_PEER_MSPCONFIGPATH=/etc/hyperledger/msp/users/Admin@org1.example.com/msp peer0.org1.example.com env TERM=${TERM} /project/src/github.com/hyperledger-labs/fabric-private-chaincode/fabric/bin/peer.sh chaincode invoke -o orderer.example.com:7050 -C mychannel -n $2 -c '{"Args":["decryptMessage", "'"$VAR"'"]}' --waitForEvent
