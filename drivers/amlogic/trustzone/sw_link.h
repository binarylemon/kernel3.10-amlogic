/*
 * Copyright (c) 2010-2013 Sierraware, LLC.
 *
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions and derivatives of the Software.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification is strictly prohibited without prior written consent from
 * Sierraware, LLC.
 *
 * Redistribution in binary form must reproduce the above copyright  notice, 
 * this list of conditions and  the following disclaimer in the documentation 
 * and/or other materials  provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 */
/* 
 * Sierra List implementation
 */

#ifndef __LIB_LINK_H__
#define __LIB_LINK_H__

#define LINK_POISON_PREV    0xEAEAEAEA
#define LINK_POISON_NEXT    0xCACACACA

#define add_link_and_data(list, head, pos, data) do { \
	set_link_data(head, data); \
	add_link(list, head, pos); \
} while(0)

/**
 * @brief 
 */
struct link {
	struct link *next, *prev;
	void *data;
};

/**
 * @brief 
 *
 * @param head
 */
static inline void link_init(struct link *head)
{
	head->next = head;
	head->prev = head;
	head->data = (void*)0;	
}

/**
 * @brief Queue interface
 *
 * @param head
 */
static inline void queue_init(struct link **head)
{
	*head = (void*)0;
}

/**
 * @brief 
 *
 * @param node
 * @param data
 */
static inline void set_link_data(struct link *node, void *data)
{
	node->data = data;
}

/**
 * @brief 
 *
 * @param head
 *
 * @return 
 */
static inline struct link * get_next_node(struct link *head)
{
	if(head->next)
		return head->next;
	else 
		return (void*) 0;
}

/**
 * @brief 
 *
 * @param head
 *
 * @return 
 */
static inline int link_empty(struct link *head)
{
	return (head->next == head);
}

#define HEAD 1
#define TAIL 2

/**
 * @brief 
 *
 * @param node
 */
static inline void remove_link(struct link *node)
{
	struct link *next, *prev;

	next = node->next;
	prev = node->prev;

	prev->next = node->next;
	next->prev = node->prev;
	node->next = (void *)LINK_POISON_NEXT;
	node->prev = (void *)LINK_POISON_PREV;
}

/**
 * @brief 
 *
 * @param head
 * @param new
 * @param pos
 */
static inline void add_link(struct link *head, struct link *new, int pos)
{
	struct link *next, *prev;
	if(pos == HEAD) {
		prev = head;
		next = head->next;
	}
	else 
	{	
		prev = head->prev;
		next = head;		
	}

	new->prev = prev;
	new->next = next;
	prev->next = new;
	next->prev = new;
}

/**
 * @brief 
 *
 * @param head
 *
 * @return 
 */
static inline struct link * pop_link(struct link *head)
{
	struct link *next;
	next = head->next;

	remove_link(next);
	return next;
}

/**
 * @brief Queue interface
 *
 * @param head
 * @param new
 */
static inline void queue_insert_tail(struct link **head, struct link *new)
{
	if (*head != (void*)0) {
		add_link(*head, new, TAIL);
	} else {
		link_init(new);
		*head = new;
	}
}

/**
 * @brief 
 *
 * @param head
 */
static inline void queue_remove_head(struct link **head)
{
	struct link *l;

	l = *head;

	if (l->next == l) {
		*head = (void*)0;
	} else {
		*head = l->next;
		remove_link(l);
	}
}

/**
 * @brief 
 *
 * @param head
 *
 * @return 
 */
static inline int queue_empty(struct link *head)
{
	return (head == (void*)0);
}


/**
 * @brief 
 *
 * @param head
 * @param data
 *
 * @return 
 */
static inline struct link *find_link(struct link *head, void *data)
{
	struct link *temp = 0;

	if(!head)
		return (void*) 0;


	if(!data)
		return (void*) 0;

	temp = head->next;

	if((unsigned int)temp == LINK_POISON_NEXT ||
			(unsigned int)temp == LINK_POISON_PREV) 
		return (void*) 0;

	while (temp != head) {
		if(temp->data) {
			if(temp->data == data)
				return temp;
		}
		temp = temp->next;
		if((unsigned int)temp == LINK_POISON_NEXT ||
				(unsigned int)temp == LINK_POISON_PREV) 
			break;
	}
	return (void*)0;
}


#endif /* __LIB_LINK_H__ */
