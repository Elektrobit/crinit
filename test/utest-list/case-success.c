/**
 * @file case-success.c
 * @brief Unit test for crinit list api.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */

#include "common.h"
#include "list.h"
#include "unit_test.h"
#include "utest-list.h"

typedef struct crinitTestEntry {
    uint8_t val;
    crinitList_t list;
} crinitTestEntry_t;

static crinitList_t crinitSl = CRINIT_LIST_INIT(crinitSl);

static int crinitCmpTestEntry(crinitList_t *e1, crinitList_t *e2) {
    crinitTestEntry_t *p1 = crinitListEntry(e1, crinitTestEntry_t, list);
    crinitTestEntry_t *p2 = crinitListEntry(e2, crinitTestEntry_t, list);

    if (p1->val > p2->val) {
        return 1;
    } else if (p1->val < p2->val) {
        return -1;
    } else {
        return 0;
    }
}

void crinitListTestSuccess(void **state) {
    CRINIT_PARAM_UNUSED(state);

    /* Test macro list initialization */
    print_message("Testing macro list initialization - CRINIT_LIST_INIT.\n");

    assert_ptr_equal(crinitSl.next, &crinitSl);
    assert_ptr_equal(crinitSl.prev, &crinitSl);

    /* Test dynamic list initialization */
    print_message("Testing dynamic list initialization - crinitListInit.\n");

    crinitList_t l1 = {0};

    assert_ptr_equal(l1.next, NULL);
    assert_ptr_equal(l1.prev, NULL);

    crinitListInit(&l1);

    assert_ptr_equal(l1.next, &l1);
    assert_ptr_equal(l1.prev, &l1);

    /* Test list is empty */
    print_message("Testing list is empty - crinitListIsEmpty.\n");

    assert_true(crinitListIsEmpty(&l1));

    /* Test list insert beginning */
    print_message("Testing list insert beginning - crinitListInsert.\n");

    crinitTestEntry_t e1 = {0}, e2 = {0};

    crinitListInsert(&e1.list, l1.prev, l1.next);

    assert_ptr_equal(l1.next, &e1.list);
    assert_ptr_equal(l1.prev, &e1.list);
    assert_ptr_equal(e1.list.prev, &l1);
    assert_ptr_equal(e1.list.next, &l1);

    crinitListInsert(&e2.list, &e1.list, e1.list.next);

    assert_ptr_equal(e1.list.next, &e2.list);
    assert_ptr_equal(l1.prev, &e2.list);
    assert_ptr_equal(e2.list.prev, &e1.list);
    assert_ptr_equal(e2.list.next, &l1);

    /* Test list append */
    print_message("Testing list append - crinitListAppend.\n");

    crinitTestEntry_t e3 = {0};

    crinitListAppend(&l1, &e3.list);

    assert_ptr_equal(e2.list.next, &e3.list);
    assert_ptr_equal(l1.prev, &e3.list);
    assert_ptr_equal(e3.list.prev, &e2.list);
    assert_ptr_equal(e3.list.next, &l1);

    /* Test list prepend */
    print_message("Testing list prepend - crinitListPrepend.\n");

    crinitTestEntry_t e4 = {0};

    crinitListPrepend(&l1, &e4.list);

    assert_ptr_equal(l1.next, &e4.list);
    assert_ptr_equal(e1.list.prev, &e4.list);
    assert_ptr_equal(e4.list.prev, &l1);
    assert_ptr_equal(e4.list.next, &e1.list);

    /* Test list delete */
    print_message("Testing list delete - crinitListDelete.\n");

    crinitListDelete(&e4.list);

    assert_ptr_equal(l1.next, &e1.list);
    assert_ptr_equal(e1.list.prev, &l1);
    assert_ptr_equal(e4.list.prev, NULL);
    assert_ptr_equal(e4.list.next, NULL);

    /* Test crinit list get container entry */
    print_message("Testing crinit list get container entry - crinitListEntry.\n");
    assert_ptr_equal(&e1, crinitListEntry(&e1.list, crinitTestEntry_t, list));
    assert_ptr_equal(&e2, crinitListEntry(&e2.list, crinitTestEntry_t, list));
    assert_ptr_equal(&e3, crinitListEntry(&e3.list, crinitTestEntry_t, list));

    /* Test crinit list get first container entry */
    print_message("Testing crinit list get first container entry - crinitListFirstEntry.\n");
    assert_ptr_equal(&e1, crinitListFirstEntry(&l1, crinitTestEntry_t, list));

    /* Test crinit list get last container entry */
    print_message("Testing crinit list get last container entry - crinitListLastEntry.\n");
    assert_ptr_equal(&e3, crinitListLastEntry(&l1, crinitTestEntry_t, list));

    /* Test crinit list get previous container entry */
    print_message("Testing crinit list get previous container entry - crinitListPrevEntry.\n");
    assert_ptr_equal(&e2, crinitListPrevEntry(&e3, list));

    /* Test crinit list get next container entry */
    print_message("Testing crinit list get next container entry - crinitListNextEntry.\n");
    assert_ptr_equal(&e2, crinitListNextEntry(&e1, list));

    /* Test crinit list is list head container entry */
    print_message("Testing crinit list is list head container entry - crinitListEntryIsHead.\n");
    assert_false(crinitListEntryIsHead(&e1, &l1, list));

    /* Test crinit list unsafe iteration */
    print_message("Testing crinit list unsafe iteration - crinitListForEachEntry.\n");
    int i = 0;
    crinitTestEntry_t *cur;
    crinitTestEntry_t *expected[] = {&e1, &e2, &e3};

    crinitListForEachEntry(cur, &l1, list) {
        assert_ptr_equal(cur, expected[i++]);
    }

    /* Test crinit list safe iteration */
    print_message("Testing crinit list safe iteration - crinitListForEachEntrySafe.\n");
    i = 0;
    crinitTestEntry_t *temp;

    crinitListForEachEntrySafe(cur, temp, &l1, list) {
        assert_ptr_equal(cur, expected[i++]);
        crinitListDelete(&cur->list);
    }

    /* Finally the list should be empty again */
    assert_true(crinitListIsEmpty(&l1));

    assert_ptr_equal(l1.next, &l1);
    assert_ptr_equal(l1.prev, &l1);

    /* Test sorted insertion */
    print_message("Testing sorted insertion - crinitListInsertSorted.\n");

    crinitListCmp_t cmp = crinitCmpTestEntry;
    crinitTestEntry_t entries[] = {
        {.val = 5}, {.val = 4}, {.val = 3}, {.val = 2}, {.val = 1},
    };

    for (size_t j = 0; j < ARRAY_SIZE(entries); j++) {
        crinitListInsertSorted(&l1, &entries[j].list, cmp);
    }

    i = 1;
    crinitListForEachEntry(cur, &l1, list) {
        assert_int_equal(cur->val, i++);
    }
}
