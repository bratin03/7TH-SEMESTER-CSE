
/*
This is a simple list implementation.
This struct can be used inside a struct as a list.
This is done for simple traversal of a list.
When we have this implementation and traverse the list using this struct, to find the
address of the different data members, we need to know the offset of each data member.
*/
struct list_head {
    struct list_head *next, *prev;
};

/*
This is a hash list implementation.
This basically points to the first pointer of the hash list
*/
struct hlist_head {
    struct hlist_node *first;
};

/*
This is a hash list node.
This basically points to the next and previous pointer of the hash list
This is specifically used for the hash list
*/
struct hlist_node {
    struct hlist_node *next, **pprev;
};

/*
Use of double pointer in linux kernel Hash list implementation:
https://stackoverflow.com/questions/3058592/use-of-double-pointer-in-linux-kernel-hash-list-implementation
*/