#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head =
        (struct list_head *) malloc(sizeof(struct list_head));
    if (head == NULL) {
        perror("malloc");
        exit(1);
    }
    INIT_LIST_HEAD(head);
    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head)
        return;

    struct list_head *pos, *safe;

    list_for_each_safe (pos, safe, head) {
        element_t *entry = list_entry(pos, element_t, list);
        list_del(pos);
        free(entry->value);  // **確保釋放 value**
        free(entry);         // **確保釋放 element_t**
    }

    free(head);  // **確保釋放 list_head**
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *new_node = malloc(sizeof(element_t));
    if (!new_node)
        return false;  // 🔴 直接返回，確保 `malloc` 失敗時不繼續執行

    new_node->value = strdup(s);
    if (!new_node->value) {
        free(new_node);  // 🔴 確保 `strdup` 失敗時釋放記憶體
        return false;
    }

    list_add(&new_node->list, head);
    return true;
}


/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *new_node = malloc(sizeof(element_t));
    if (!new_node)
        return false;  // 🔴 確保 malloc 失敗時直接返回

    new_node->value = strdup(s);
    if (!new_node->value) {
        free(new_node);  // 🔴 確保 `strdup` 失敗時釋放記憶體
        return false;
    }

    list_add_tail(&new_node->list, head);
    return true;
}


/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (list_empty(head))
        return NULL;

    element_t *entry = list_first_entry(head, element_t, list);
    list_del(&entry->list);

    if (sp) {
        strncpy(sp, entry->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }

    return entry;  // 讓呼叫者負責釋放記憶體
}



/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (list_empty(head)) {
        return NULL;
    }
    element_t *entry = list_last_entry(head, element_t, list);
    list_del(&entry->list);
    if (sp != NULL) {
        strncpy(sp, entry->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    return entry;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    int count = 0;
    struct list_head *pos;
    list_for_each (pos, head) {
        count++;
    }
    return count;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    if (list_empty(head))
        return false;
    struct list_head *slow = head->next, *fast = head->next;
    while (fast != head && fast->next != head) {
        fast = fast->next->next;
        slow = slow->next;
    }
    element_t *mid = list_entry(slow, element_t, list);
    list_del(&mid->list);
    free(mid->value);
    free(mid);
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    if (list_empty(head))
        return false;
    element_t *cur, *safe;
    bool deleted = false;
    list_for_each_entry_safe (cur, safe, head, list) {
        if (&safe->list != head && strcmp(cur->value, safe->value) == 0) {
            list_del(&cur->list);
            free(cur->value);
            free(cur);
            deleted = true;
        }
    }
    return deleted;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    struct list_head *node;
    list_for_each (node, head) {
        if (node->next == head)
            break;
        list_move(node, node->next);
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    struct list_head *pos, *safe;

    list_for_each_safe (pos, safe, head) {
        list_move(pos, head);
    }
}


/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    if (k < 2 || list_empty(head))
        return;
    struct list_head *cur = head->next, *next, *tail;
    int count = 0;
    while (cur != head) {
        count++;
        cur = cur->next;
    }
    while (count >= k) {
        cur = head->next;
        tail = cur;
        for (int i = 0; i < k; i++)
            tail = tail->next;
        for (int i = 0; i < k; i++) {
            next = cur->next;
            list_move(cur, tail);
            cur = next;
        }
        count -= k;
    }
}

/* Sort elements of queue in ascending/descending order */
static struct list_head *merge_sorted_lists(struct list_head *l1,
                                            struct list_head *l2,
                                            bool descend)
{
    struct list_head dummy;
    INIT_LIST_HEAD(&dummy);
    struct list_head *tail = &dummy;

    // fprintf(stderr, "[merge_sorted_lists] Start merging\n");

    while (!list_empty(l1) && !list_empty(l2)) {
        element_t *e1 = list_first_entry(l1, element_t, list);
        element_t *e2 = list_first_entry(l2, element_t, list);

        // fprintf(stderr, "[merge_sorted_lists] Comparing '%s' and '%s'\n",
        //         e1->value, e2->value);

        if (strcmp(e1->value, e2->value) < 0 ||
            (strcmp(e1->value, e2->value) == 0 &&
             e1 < e2))  // ✅ 確保 e1 來自 l1
            list_move_tail(&e1->list, tail);
        else
            list_move_tail(&e2->list, tail);
    }

    if (!list_empty(l1)) {
        // fprintf(stderr,
        //         "[merge_sorted_lists] Appending remaining elements from
        //         l1\n");
        list_splice_tail_init(l1, tail);
    }
    if (!list_empty(l2)) {
        // fprintf(stderr,
        //         "[merge_sorted_lists] Appending remaining elements from
        //         l2\n");
        list_splice_tail_init(l2, tail);
    }

    // fprintf(stderr, "[merge_sorted_lists] Final merged list:\n");
    // struct list_head *pos;
    // int final_count = 0;
    // list_for_each (pos, &dummy) {
    //     element_t *entry = list_entry(pos, element_t, list);
    //     fprintf(stderr, "  Node %d: %p | Value: %s\n", final_count++, entry,
    //             entry->value ? entry->value : "NULL");
    // }
    // fprintf(stderr, "[merge_sorted_lists] Final merged list count: %d\n",
    //         final_count);

    list_splice_tail_init(&dummy, l1);  // **把 dummy 的內容搬回 l1**
    return l1;
}


void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    struct list_head left, right;
    INIT_LIST_HEAD(&left);
    INIT_LIST_HEAD(&right);

    struct list_head *curr = head->next;
    bool toggle = false;

    // **使用 alternate split 來確保 stable sort**
    while (curr != head) {
        struct list_head *next = curr->next;
        list_del_init(curr);  // **刪除當前節點**
        if (toggle)
            list_add_tail(curr, &left);
        else
            list_add_tail(curr, &right);
        toggle = !toggle;
        curr = next;  // **更新 curr**
    }

    if (!list_empty(&left))
        q_sort(&left, descend);
    if (!list_empty(&right))
        q_sort(&right, descend);

    // **Merge 回來**
    struct list_head *sorted = merge_sorted_lists(&left, &right, descend);

    if (sorted && !list_empty(sorted)) {
        INIT_LIST_HEAD(head);
        list_splice_tail(sorted, head);
    }
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    if (list_empty(head))
        return 0;
    element_t *cur, *safe;
    list_for_each_entry_safe (cur, safe, head, list) {
        if (strcmp(cur->value, safe->value) > 0) {
            list_del(&cur->list);
            free(cur->value);
            free(cur);
        }
    }
    return q_size(head);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    if (list_empty(head))
        return 0;
    element_t *cur, *safe;
    list_for_each_entry_safe (cur, safe, head, list) {
        if (strcmp(cur->value, safe->value) < 0) {
            list_del(&cur->list);
            free(cur->value);
            free(cur);
        }
    }
    return q_size(head);
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    if (list_empty(head))
        return 0;

    struct list_head *pos, *safe;
    struct list_head *merged_list = NULL;

    // int queue_cnt = 0;

    // list_for_each_safe (pos, safe, head) {
    //     queue_contex_t *context = list_entry(pos, queue_contex_t, chain);
    //     struct list_head *queue = context->q;  // 取得 queue (list of
    //     element_t)

    //     fprintf(stderr, "[q_merge] Checking queue before merge: %d\n",
    //             ++queue_cnt);

    //     struct list_head *q_pos, *q_safe;
    //     list_for_each_safe (q_pos, q_safe, queue) {
    //         element_t *entry = list_entry(q_pos, element_t, list);
    //         // check queue
    //         fprintf(stderr, "[q_merge] Checking queue element: %s\n",
    //                 entry->value);
    //     }
    // }

    // **先把所有 queue_contex_t 的 queue 取出來**
    list_for_each_safe (pos, safe, head) {
        queue_contex_t *context = list_entry(pos, queue_contex_t, chain);
        struct list_head *queue = context->q;  // 取得 queue (list of element_t)

        if (!merged_list) {
            merged_list = queue;  // **第一條 queue，直接存起來**
        } else {
            merged_list = merge_sorted_lists(merged_list, queue, descend);
            // list_del(&context->chain);
            // q_free(queue);
        }
    }

    // queue_cnt = 0;

    // list_for_each_safe (pos, safe, head) {
    //     queue_contex_t *context = list_entry(pos, queue_contex_t, chain);
    //     struct list_head *queue = context->q;  // 取得 queue (list of
    //     element_t)

    //     fprintf(stderr, "[q_merge] Checking queue after merge: %d\n",
    //             ++queue_cnt);

    //     struct list_head *q_pos, *q_safe;
    //     list_for_each_safe (q_pos, q_safe, queue) {
    //         element_t *entry = list_entry(q_pos, element_t, list);
    //         // check queue
    //         fprintf(stderr, "[q_merge] Checking queue element: %s\n",
    //                 entry->value);
    //     }
    // }

    int final_size = q_size(merged_list);
    // fprintf(stderr, "[DEBUG] Final queue size: %d\n", final_size);
    return final_size;
}
