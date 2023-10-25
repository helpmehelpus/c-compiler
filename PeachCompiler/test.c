struct book {
    char name[30];
};

int test(char* fmt)
{
    return 1;
}

struct book book;

int main() {
    struct book* books;
    return test(65, books[0].name, 1000);
}

/*
 * root exp
 *
 * books[0] is left expression
 *
 * .name is right expression -> name is left node, right node is 1000; operator is . (dot)
 *
 * Initially read as books[0].(name, 1000). We need to reorder the nodes
 *
 * After call to parser_node_move_right_left_to_left, name will now be in the left-hand side: (books[0].name, 1000)
 *
 *
 */