#include <stdio.h>
#include <stdlib.h>
#include <check.h>
#include <assert.h>

#include "IndexTree.h"

class data {
public:
	long	key;
	data(long k) : key(k) { };
};

class foobar : public IndexTree<data> {
	int Cmp(const data *a, const data *b) const {
		if (a->key == b->key)
			return 0;
		if (a->key < b->key)
			return -1;
		return 1;
	}
};


START_TEST(indextree)
#define NENT	10000
{
	foobar	tree;
	data *e[NENT];
	int cnt = 0, tries;
	int i;

	tree.verify();
	for (i=0; i<NENT; ++i) {
		e[i] = new data(-1);
	}
	for (tries = 0; cnt || tries < 1000000; ++tries) {
		int idx = rand() % NENT;
		if (tries > 1000000)
			for (; e[idx]->key == -1; idx = (idx+1) % NENT) ;
		if (e[idx]->key == -1) {
			e[idx]->key = idx;
			assert(tree.insert(e[idx]));
			cnt++;
		} else {
			assert(tree.remove(e[idx]));
			e[idx]->key = -1;
			cnt--;
		}
		if (cnt < 1000 || tries % 100 == 0) {
			tree.verify();
			assert(tree.size() == cnt);
			foobar::iterator it;
			int tmp;
			for (it = tree.begin(), tmp = 0;
			    it != tree.end(); ++it, ++tmp) {
				assert(tree.index_of(*it) == tmp);
			}
		}
	}
}
END_TEST

TCase *
getTc_IndexTree(void)
{
	TCase *testCase;
	testCase = tcase_create("indextree");
	tcase_set_timeout(testCase, 60);
	tcase_add_test(testCase, indextree);

	return testCase;
}
