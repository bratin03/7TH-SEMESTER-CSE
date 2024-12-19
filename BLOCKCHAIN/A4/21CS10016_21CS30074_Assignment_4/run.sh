sudo ./network.sh down
sudo ./network.sh up createChannel -ca -c mychannel
sudo ./network.sh deployCC -ccn mychaincode -ccp ../../chaincode -ccl javascript -cccg ../../chaincode/collections_config.json -ccep "OR('Org1MSP.peer','Org2MSP.peer')"
