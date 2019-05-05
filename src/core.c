#include <stdio.h>

#include "trie.h"

int visitor_print2(const char *key, void *data, void *arg)
{
    (void) data;
    (void) arg;
    printf("%s\n", key);
    return 0;
}

int main2()
{
    struct trie *t = trie_create();

    (void) trie_insert(t, (const char *) "key1", (void *) "data1");
    (void) trie_insert(t, (const char *) "key2", (void *) "data2");
    (void) trie_insert(t, (const char *) "key3", (void *) "data3");
    (void) trie_insert(t, (const char *) "key4", (void *) "data4");
    (void) trie_insert(t, (const char *) "key5", (void *) "data5");
    (void) trie_insert(t, (const char *) "key6", (void *) "data6");
    (void) trie_insert(t, (const char *) "key7", (void *) "data7");
    (void) trie_insert(t, (const char *) "key8", (void *) "data8");
    (void) trie_insert(t, (const char *) "key9", (void *) "data9");
    (void) trie_insert(t, (const char *) "key10", (void *) "data10");
    (void) trie_insert(t, (const char *) "key11", (void *) "data11");
    (void) trie_insert(t, (const char *) "key12", (void *) "data12");
    (void) trie_insert(t, (const char *) "key13", (void *) "data13");
    (void) trie_insert(t, (const char *) "key14", (void *) "data14");
    (void) trie_insert(t, (const char *) "key15", (void *) "data15");
    (void) trie_insert(t, (const char *) "key16", (void *) "data16");
    (void) trie_insert(t, (const char *) "key17", (void *) "data17");
    (void) trie_insert(t, (const char *) "key18", (void *) "data18");
    (void) trie_insert(t, (const char *) "key19", (void *) "data19");
    (void) trie_insert(t, (const char *) "key20", (void *) "data20");
    (void) trie_insert(t, (const char *) "key21", (void *) "data21");
    (void) trie_insert(t, (const char *) "key22", (void *) "data22");
    (void) trie_insert(t, (const char *) "key23", (void *) "data23");
    (void) trie_insert(t, (const char *) "key24", (void *) "data24");
    (void) trie_insert(t, (const char *) "key25", (void *) "data25");
    (void) trie_insert(t, (const char *) "key26", (void *) "data26");
    (void) trie_insert(t, (const char *) "key27", (void *) "data27");
    (void) trie_insert(t, (const char *) "key28", (void *) "data28");
    (void) trie_insert(t, (const char *) "key29", (void *) "data29");
    (void) trie_insert(t, (const char *) "key30", (void *) "data30");
    (void) trie_insert(t, (const char *) "key31", (void *) "data31");
    (void) trie_insert(t, (const char *) "key32", (void *) "data32");
    (void) trie_insert(t, (const char *) "key33", (void *) "data33");
    (void) trie_insert(t, (const char *) "key34", (void *) "data34");
    (void) trie_insert(t, (const char *) "key35", (void *) "data35");
    (void) trie_insert(t, (const char *) "key36", (void *) "data36");
    (void) trie_insert(t, (const char *) "key37", (void *) "data37");
    (void) trie_insert(t, (const char *) "key38", (void *) "data38");
    (void) trie_insert(t, (const char *) "key39", (void *) "data39");
    (void) trie_insert(t, (const char *) "key40", (void *) "data40");
    (void) trie_insert(t, (const char *) "key41", (void *) "data41");
    (void) trie_insert(t, (const char *) "key42", (void *) "data42");
    (void) trie_insert(t, (const char *) "key43", (void *) "data43");
    (void) trie_insert(t, (const char *) "key44", (void *) "data44");
    (void) trie_insert(t, (const char *) "key45", (void *) "data45");
    (void) trie_insert(t, (const char *) "key46", (void *) "data46");
    (void) trie_insert(t, (const char *) "key47", (void *) "data47");
    (void) trie_insert(t, (const char *) "key48", (void *) "data48");
    (void) trie_insert(t, (const char *) "key49", (void *) "data49");
    (void) trie_insert(t, (const char *) "key50", (void *) "data50");

    trie_visit(t, "key", visitor_print2, NULL);

    printf("count: %zu\tsize: %zu K\n", trie_count(t, ""), trie_size(t) / 1000);

    trie_free(t);

    return 0;
}
