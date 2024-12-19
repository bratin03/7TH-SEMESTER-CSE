// SPDX-License-Identifier: UNLICENSED
pragma solidity >=0.5.8;

contract NameRollRegistry {
    struct Info {
        string name;
        string roll;
    }

    mapping(address => Info) public info;

    // Function to update both name and roll number
    function update(string memory newName, string memory newRoll) public {
        info[msg.sender].name = newName;
        info[msg.sender].roll = newRoll;
    }

    // Function to get name and roll number by address
    function get(
        address addr
    ) public view returns (string memory, string memory) {
        return (info[addr].name, info[addr].roll);
    }

    // Function to get your own name and roll number
    function getmine() public view returns (string memory, string memory) {
        return (info[msg.sender].name, info[msg.sender].roll);
    }
}
