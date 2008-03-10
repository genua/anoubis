/*
 * Copyright (c) 2008 GeNUA mbH <info@genua.de>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <config.h>
#include <check.h>

#include <anoubischat_dummy.h>

#include <anoubis_protocol.h>
#include <anoubis_errno.h>
#include <anoubis_client.h>
#include <anoubis_server.h>
#include <anoubis_msg.h>
#include <anoubis_transaction.h>
#include <anoubis_notify.h>
#include <anoubis_dump.h>

#define NRCLIENT 32
#define UID	4711
#define TASK	12345
#define RULE	67890
#define SUBSYS	321

#define UIDX	0
#define TIDX	1
#define	RIDX	2
#define	SIDX	3

START_TEST(tp_notify_reg)
{
	struct achat_channel * chan[NRCLIENT];
	struct anoubis_notify_group * ng[NRCLIENT];
	struct anoubis_msg * m;
	char buf[4096];
	int token = 0x123000;
	int i, ret;
	int cnt[NRCLIENT];

	for (i=0; i<NRCLIENT; ++i) {
		chan[i] = acc_create();
		fail_if(chan[i] == NULL, "failed to create channel");
		ng[i] = anoubis_notify_create(chan[i], UID + i%2);
		fail_if(ng[i] == NULL);
		ret = anoubis_notify_register(ng[i], UID + i%2, (i&2)?TASK:0,
		    (i&4)?RULE:0, (i&8)?SUBSYS:0);
		fail_if(ret != 0, "Register failed with %d", ret);
		if (i&16) {
			ret = anoubis_notify_register(ng[i], UID + i%2,
			    (i&2)?TASK+1:0, (i&4)?RULE+1:0, (i&8)?SUBSYS+1:0);
			fail_if(ret != 0, "Register failed with %d", ret);
		}
		cnt[i] = 0;
	}
	for (i=0; i<81; ++i) {
		struct anoubis_notify_head * head;
		int off[4], j, k;
		k=i;
		for(j=0; j<4; ++j) {
			off[j] = -1+k%3;
			k/=3;
		}
		m = anoubis_msg_new(sizeof(Anoubis_NotifyMessage));
		fail_if(m == NULL, "Cannot allocate message");
		set_value(m->u.notify->type, ANOUBIS_N_NOTIFY);
		set_value(m->u.notify->pid, TASK+off[TIDX]);
		set_value(m->u.notify->uid, UID+off[UIDX]);
		set_value(m->u.notify->subsystem, SUBSYS+off[SIDX]);
		set_value(m->u.notify->rule_id, RULE+off[RIDX]);
		m->u.notify->token = ++token;
		head = anoubis_notify_create_head(TASK+off[TIDX],
		    m, NULL, NULL);
		for(j=0; j<NRCLIENT; ++j) {
			ret = anoubis_notify(ng[j], head);
			fail_if(ret < 0, "notify failed with %d", ret);
			if (ret)
				cnt[j]++;
		}
		anoubis_notify_destroy_head(head);
	}
	for (i=0; i<NRCLIENT; ++i) {
		int expect = 1;
		int real;
		if ((i&16) && (i&14))
			expect++;
		if ((i&2) == 0)
			expect *= 3;
		if ((i&4) == 0)
			expect *= 3;
		if ((i&8) == 0)
			expect *= 3;
		real = 0;
		while(1) {
			int ret;
			size_t len;
			len = 4096;
			ret = acc_receivemsg_operator(chan[i], buf, &len);
			if (ret == ACHAT_RC_OK) {
				real++;
				continue;
			}
			if (ret == ACHAT_RC_EOF)
				break;
			fail_if(ret != ACHAT_RC_OK,
			    "receivemsg failed with %d", ret);
		}
		fail_if(expect != real, "Wrong number of messages for %d "
		    "real = %d expected = %d", i, real, expect);
	}
	for (i=0; i<NRCLIENT; ++i) {
		anoubis_notify_destroy(ng[i]);
		acc_destroy(chan[i]);
	}
}
END_TEST

#undef NRCLIENT
#define NRCLIENT 4
#define TOKENOFF 0x234000

START_TEST(tp_ask)
{
	struct achat_channel * chan[NRCLIENT];
	struct anoubis_notify_group * ng[NRCLIENT];
	struct anoubis_msg * m;
	struct anoubis_notify_head * heads[10];
	int i, k, ret;

	for (i=0; i<NRCLIENT; ++i) {
		chan[i] = acc_create();
		fail_if(chan[i] == NULL, "failed to create channel");
		ng[i] = anoubis_notify_create(chan[i], UID);
		fail_if(ng[i] == NULL);
		ret = anoubis_notify_register(ng[i], UID, 0, 0, 0);
		fail_if(ret != 0, "Register failed with %d", ret);
	}
	for (k=0; k<5; ++k) {
		m = anoubis_msg_new(sizeof(Anoubis_NotifyMessage));
		fail_if(m == NULL, "Cannot allocate message");
		set_value(m->u.notify->type, ANOUBIS_N_ASK);
		set_value(m->u.notify->uid, UID);
		set_value(m->u.notify->pid, 0x100+i);
		set_value(m->u.notify->subsystem, 0x200+i);
		set_value(m->u.notify->rule_id, 0x300+i);
		m->u.notify->token = TOKENOFF+k;
		heads[k] = anoubis_notify_create_head(0x100+i, m, NULL, NULL);
		for(i=0; i<NRCLIENT; ++i) {
			int ret = anoubis_notify(ng[i], heads[k]);
			fail_if(ret != 1, "Did not notify (%d)", ret);
		}
	}
	/* Simulate a terminating session. */
	anoubis_notify_destroy(ng[NRCLIENT-1]);
	/* Event 0: verdict = 0, RESOTHER for all peers, uid 0. */
	ret = anoubis_notify_sendreply(heads[0], 0, NULL, 0);
	fail_if(ret != 0, "sendreply for 0 failed");
	/* Event 1: verdict = 10, RESOTHER for all peers, uid 123. */
	ret = anoubis_notify_sendreply(heads[1], 10, NULL, 123);
	fail_if(ret != 0, "sendreply for 1 failed");
	/* Event 2: verdict = 20, uid = 4711, decided by client 1
	 *   - Delegate by client 2
	 *   - Answer by client 1 (wins)
	 *   - Answer by client 0 (loses)
	 *   - Second answer for each client fails.
	 */
	ret = anoubis_notify_answer(ng[2], TOKENOFF+2, 10, 1);
	fail_if(ret != 0, "first answer failed with %d", ret);
	ret = anoubis_notify_answer(ng[1], TOKENOFF+2, 20, 0);
	fail_if(ret != 0, "second answer failed");
	ret = anoubis_notify_answer(ng[0], TOKENOFF+2, 30, 0);
	fail_if(ret < 0, "third answer failed");
	for (i=0; i<NRCLIENT-1; ++i) {
		fail_if(anoubis_notify_answer(ng[i], TOKENOFF+2, 0, 0) >= 0,
		    "Duplicate notify succeeds?");
	}
	/* Event 3: verdict = 0, RESYOU for client 2, uid = 4711 */
	for (i=0; i<NRCLIENT-1; ++i) {
		ret = anoubis_notify_answer(ng[i], TOKENOFF+3, 0, 1);
		fail_if(ret < 0, "Delegate failed");
	}
	/*
	 * Event 4: verdict = 456, uid = 4711, decided by client 2
	 *    - Client 2 delegates with code 456.
	 *    - All other clients terminate which means that client 2 wins.
	 */
	ret = anoubis_notify_answer(ng[NRCLIENT-2], TOKENOFF+4, 456, 1);
	fail_if(ret < 0, "Delegate failed");
	for (i=0; i<NRCLIENT-1; ++i) {
		anoubis_notify_destroy(ng[i]);
	}
	/*
	 * Client 3: should only receive the notifies
	 * Client 2: should receive replies for event 0 to 4
	 * Client 1 and 0 should receive replies for events 0 to 3
	 */
	int decider[5] = { -1, -1, 1, 2, 2 };
	int verdicts[5] = { 0, 10, 20, 0, 456 };
	int uids[5] = { 0, 123, 4711, 4711, 4711 };
	int get[4] = { 0x0f, 0x0f, 0x1f, 0x0 };
	for (i=0; i<NRCLIENT; ++i) {
		int notify, reply;

		notify = reply = 0;
		m = anoubis_msg_new(4096);
		while(1) {
			int opcode, verdict, uid, mask,  ret;
			size_t len;
			anoubis_token_t token;

			anoubis_msg_resize(m, 4096);
			len = 4096;
			ret = acc_receivemsg_operator(chan[i], m->u.buf, &len);
			if (ret == ACHAT_RC_EOF)
				break;
			fail_if(ret != ACHAT_RC_OK,
			    "receivemsg failed with %d",  ret);
			anoubis_msg_resize(m, len);
			anoubis_dump(m, "MSG:");
			fail_if(!VERIFY_FIELD(m, token, token),
			    "Short message");
			opcode = get_value(m->u.token->type);
			token = m->u.token->token;
			token -= TOKENOFF;
			mask = (1<<token);
			if (opcode == ANOUBIS_N_ASK) {
				fail_if(notify & mask, "Duplicate notify");
				notify |= mask;
				continue;
			}
			fail_if(opcode != ANOUBIS_N_RESYOU &&
			    opcode != ANOUBIS_N_RESOTHER, "Bad opcode");
			fail_if(reply & mask, "Duplicate result?");
			reply |= mask;
			fail_if(opcode == ANOUBIS_N_RESYOU
			    && decider[token] != i, "Bad RESYOU");
			fail_if(opcode == ANOUBIS_N_RESOTHER
			    && decider[token] == i, "Bad RESOTHER");
			ret = VERIFY_LENGTH(m,
			    sizeof(Anoubis_NotifyResultMessage));
			fail_if(!ret, "Short message");
			uid = get_value(m->u.notifyresult->uid);
			fail_if(uid != uids[token],
			    "Wrong uid %d should be %d", uid, uids[token]);
			verdict = get_value(m->u.notifyresult->error);
			fail_if(verdict != verdicts[token], "Wrong verict "
			    "%d should be %d", verdict, verdicts[token]);
		}
		anoubis_msg_free(m);
		fail_if(notify != 0x1f, "Missing ask events"
		    "got 0x%02x expected 0x%02x ", notify, 0x1f);
		fail_if(reply != get[i], "Missing replies "
		    "got 0x%02x expected 0x%02x ", reply, get[i]);
	}
	for (i=0; i<NRCLIENT; ++i) {
		acc_destroy(chan[i]);
	}
}
END_TEST

TCase *
libanoubisproto_testcase_notify(void)
{
	/* Core test case */
	TCase *tp_notify = tcase_create("Core");

	tcase_add_test(tp_notify, tp_notify_reg);
	tcase_add_test(tp_notify, tp_ask);
	tcase_set_timeout(tp_notify, 30);

	return (tp_notify);
}
