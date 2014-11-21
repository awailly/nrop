/*
 * Copyright (C) 2007-2008 Tobias Brunner
 * Copyright (C) 2005-2006 Martin Willi
 * Copyright (C) 2005 Jan Hutter
 * Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include <stdlib.h>
#include <chain.h>

#include "linked_list.h"

typedef struct element_t element_t;

/**
 * This element holds a pointer to the value it represents.
 */
struct element_t {

	/**
	 * Value of a list item.
	 */
	u_char *value;

	/**
	 * Previous list element.
	 *
	 * NULL if first element in list.
	 */
	element_t *previous;

	/**
	 * Next list element.
	 *
	 * NULL if last element in list.
	 */
	element_t *next;
};

/**
 * Creates an empty linked list object.
 */
element_t *element_create(void *value)
{
	element_t *this = malloc_thing(element_t);

	this->previous = NULL;
	this->next = NULL;
	this->value = value;

	return (this);
}


typedef struct private_linked_list_t private_linked_list_t;

/**
 * Private data of a linked_list_t object.
 *
 */
struct private_linked_list_t {
	/**
	 * Public part of linked list.
	 */
	linked_list_t public;

	/**
	 * Number of items in the list.
	 */
	int count;

	/**
	 * First element in list.
	 * NULL if no elements in list.
	 */
	element_t *first;

	/**
	 * Last element in list.
	 * NULL if no elements in list.
	 */
	element_t *last;
};

typedef struct private_enumerator_t private_enumerator_t;

/**
 * linked lists enumerator implementation
 */
struct private_enumerator_t {

	/**
	 * implements enumerator interface
	 */
	enumerator_t enumerator;

	/**
	 * associated linked list
	 */
	private_linked_list_t *list;

	/**
	 * current item
	 */
	element_t *current;

    /**
     * enumerator has enumerated all items
     */
    bool finished;
};

/**
 * Implementation of private_enumerator_t.enumerator.enumerate.
 */
static bool enumerate(private_enumerator_t *this, void **item)
{
	if (!this->current)
	{
		if (!this->list->first)
		{
			return FALSE;
		}
		this->current = this->list->first;
	}
	else
	{
		if (!this->current->next)
		{
			return FALSE;
		}
		this->current = this->current->next;
	}
	*item = this->current->value;
	return TRUE;
}

/**
 * Implementation of linked_list_t.create_enumerator.
 */
static enumerator_t* create_enumerator(private_linked_list_t *this)
{
	private_enumerator_t *enumerator = malloc_thing(private_enumerator_t);

	enumerator->enumerator.enumerate = (bool (*)(enumerator_t *, ...))enumerate;
	enumerator->enumerator.destroy = (void (*)(enumerator_t *))free;
	enumerator->list = this;
	enumerator->current = NULL;

	return &enumerator->enumerator;
}

/**
 * Implementation of linked_list_t.get_count.
 */
static int get_count(private_linked_list_t *this)
{
	return this->count;
}

/**
 * Implementation of linked_list_t.insert_first.
 */
static void insert_first(private_linked_list_t *this, void *item)
{
	element_t *element;

	element = element_create(item);
	if (this->count == 0)
	{
		/* first entry in list */
		this->first = element;
		this->last = element;
		element->previous = NULL;
		element->next = NULL;
	}
	else
	{
		element_t *old_first_element = this->first;
		element->next = old_first_element;
		element->previous = NULL;
		old_first_element->previous = element;
		this->first = element;
	}
	this->count++;
}

/**
 * unlink an element form the list, returns following element
 */
static element_t* remove_element(private_linked_list_t *this, element_t *element)
{
	element_t *next, *previous;

	next = element->next;
	previous = element->previous;
	free(element);
	if (next)
	{
		next->previous = previous;
	}
	else
	{
		this->last = previous;
	}
	if (previous)
	{
		previous->next = next;
	}
	else
	{
		this->first = next;
	}
	if (--this->count == 0)
	{
		this->first = NULL;
		this->last = NULL;
	}
	return next;
}

/**
 * Implementation of linked_list_t.get_first.
 */
static status_t get_first(private_linked_list_t *this, void **item)
{
	if (this->count == 0)
	{
		return NOT_FOUND;
	}
	*item = this->first->value;
	return SUCCESS;
}

/**
 * Implementation of linked_list_t.remove_first.
 */
static status_t remove_first(private_linked_list_t *this, void **item)
{
	if (get_first(this, item) == SUCCESS)
	{
		remove_element(this, this->first);
		return SUCCESS;
	}
	return NOT_FOUND;
}

/**
 * Implementation of linked_list_t.insert_last.
 */
static void insert_last(private_linked_list_t *this, void *item)
{
	element_t *element = element_create(item);

	if (this->count == 0)
	{
		/* first entry in list */
		this->first = element;
		this->last = element;
		element->previous = NULL;
		element->next = NULL;
	}
	else
	{
		element_t *old_last_element = this->last;
		element->previous = old_last_element;
		element->next = NULL;
		old_last_element->next = element;
		this->last = element;
	}
	this->count++;
}

/**
 * Implementation of linked_list_t.insert_before.
 */
static void insert_before(private_linked_list_t *this, private_enumerator_t *enumerator, void *item)
{
    element_t *current, *element;

    current = enumerator->current;
    if (!current)
    {
        if (enumerator->finished)
        {
            this->public.insert_last(&this->public, item);
        }
        else
        {
            this->public.insert_first(&this->public, item);
        }
        return;
    }
    element = element_create(item);
    if (current->previous)
    {
        current->previous->next = element;
        element->previous = current->previous;
        current->previous = element;
        element->next = current;
    }
    else
    {
        current->previous = element;
        element->next = current;
        this->first = element;
    }
    this->count++;
}


/**
 * Implementation of linked_list_t.get_last.
 */
static status_t get_last(private_linked_list_t *this, void **item)
{
	if (this->count == 0)
	{
		return NOT_FOUND;
	}
	*item = this->last->value;
	return SUCCESS;
}

/**
 * Implementation of linked_list_t.remove_last.
 */
static status_t remove_last(private_linked_list_t *this, void **item)
{
	if (get_last(this, item) == SUCCESS)
	{
		remove_element(this, this->last);
		return SUCCESS;
	}
	return NOT_FOUND;
}

/**
 * Implementation of linked_list_t.remove.
 */
static int remove_(private_linked_list_t *this, void *item,
				   bool (*compare)(void *,void*))
{
	element_t *current = this->first;
	int removed = 0;

	while (current)
	{
		if ((compare && compare(current->value, item)) ||
			(!compare && current->value == item))
		{
			removed++;
			current = remove_element(this, current);
		}
		else
		{
			current = current->next;
		}
	}
	return removed;
}

/**
 * Implementation of linked_list_t.remove_at.
 */
static void remove_at(private_linked_list_t *this, private_enumerator_t *enumerator)
{
	element_t *current;

	if (enumerator->current)
	{
		current = enumerator->current;
		enumerator->current = current->previous;
		remove_element(this, current);
	}
}

/**
 * Implementation of linked_list_t.find_first.
 */
static status_t find_first(private_linked_list_t *this, linked_list_match_t match,
		void **item, void *d1, void *d2, void *d3, void *d4, void *d5)
{
	element_t *current = this->first;

	while (current)
	{
		if ((match && match(current->value, d1, d2, d3, d4, d5)) ||
			(!match && item && current->value == *item))
		{
			if (item != NULL)
			{
				*item = current->value;
			}
			return SUCCESS;
		}
		current = current->next;
	}
	return NOT_FOUND;
}

/**
 * Implementation of linked_list_t.find_last.
 */
static status_t find_last(private_linked_list_t *this, linked_list_match_t match,
		void **item, void *d1, void *d2, void *d3, void *d4, void *d5)
{
	element_t *current = this->last;

	while (current)
	{
		if ((match && match(current->value, d1, d2, d3, d4, d5)) ||
			(!match && item && current->value == *item))
		{
			if (item != NULL)
			{
				*item = current->value;
			}
			return SUCCESS;
		}
		current = current->previous;
	}
	return NOT_FOUND;
}

/**
 * Implementation of linked_list_t.invoke_offset.
 */
static void invoke_offset(private_linked_list_t *this, size_t offset,
		void *d1, void *d2, void *d3, void *d4, void *d5)
{
	element_t *current = this->first;

	while (current)
	{
		linked_list_invoke_t *method = (linked_list_invoke_t *) current->value + offset;
		(*method)(current->value, d1, d2, d3, d4, d5);
		current = current->next;
	}
}

/**
 * Implementation of linked_list_t.invoke_function.
 */
static void invoke_function(private_linked_list_t *this, linked_list_invoke_t fn,
		void *d1, void *d2, void *d3, void *d4, void *d5)
{
	element_t *current = this->first;

	while (current)
	{
		fn(current->value, d1, d2, d3, d4, d5);
		current = current->next;
	}
}

/**
 * Implementation of linked_list_t.clone_offset
 */
static linked_list_t *clone_offset(private_linked_list_t *this, size_t offset)
{
	linked_list_t *clone = linked_list_create();
	element_t *current = this->first;

	while (current)
	{
		void* (**method)(void*) = (void *(**)(void*)) current->value + offset;
		clone->insert_last(clone, (*method)(current->value));
		current = current->next;
	}

	return clone;
}

/**
 * Implementation of linked_list_t.clone_function
 */
static linked_list_t *clone_function(private_linked_list_t *this, void* (*fn)(void*))
{
	linked_list_t *clone = linked_list_create();
	element_t *current = this->first;

	while (current)
	{
		clone->insert_last(clone, fn(current->value));
		current = current->next;
	}

	return clone;
}

/**
 * Implementation of linked_list_t.sort
 */
static linked_list_t *sort(private_linked_list_t *this)
{
    linked_list_t *inst_list_sorted;

    enumerator_t *e;
    chain_t *c;

    inst_list_sorted = linked_list_create();

    e = this->public.create_enumerator(&this->public);

    while(e->enumerate(e, &c))
    {
        if (inst_list_sorted->get_count(inst_list_sorted) == 0)
        {
            inst_list_sorted->insert_last(inst_list_sorted, c);
        }
        else
        {
            enumerator_t *e_sorted;
            chain_t *c_sorted;

            e_sorted = inst_list_sorted->create_enumerator(inst_list_sorted);
            c_sorted = c;

            while ((strcmp(c->get_str(c), c_sorted->get_str(c_sorted)) >= 0) && (e_sorted->enumerate(e_sorted, &c_sorted)));

            inst_list_sorted->insert_before(inst_list_sorted, e_sorted, c);

            e_sorted->destroy(e_sorted);
        }
    }

    e->destroy(e);

    return inst_list_sorted;
}

static int compare(element_t *a, element_t *b)
{
    chain_t *c, *d;

    c = (chain_t*) a->value;
    d = (chain_t*) b->value;

    return strcmp(c->get_str(c), d->get_str(d));
}

static element_t *bsort_rec(element_t *list)
{
    element_t *right=list,*temp=list,*last=list,*result=0,*next=0,*tail=0;

    if (!list || !list->next) return list;  // Trivial case.

    // Find halfway through the list (by running two pointers, one at twice the speed of the other).
    while (temp && temp->next) {last=right;right=right->next,temp=temp->next->next;}

    last->next=0;   // Break the list in two. (prev pointers are broken here, but we fix later)

    // Recurse on the two smaller lists:
    list=bsort_rec(list);
    right=bsort_rec(right);

    // Merge:
    while (list || right)
    {
        // Take from empty lists, or compare:
        if (!right)                     {next=list;list=list->next;}
        else if (!list)                 {next=right;right=right->next;}
        else if (compare(list,right)<0) {next=list;list=list->next;}
        else                            {next=right;right=right->next;}
        if (!result) result=next; else tail->next=next;
        next->previous=tail;    // Optional.
        tail=next;
    }
    return result;
}

/**
 * Implementation of linked_list_t.bsort
 */
static linked_list_t *bsort(private_linked_list_t *this)
{
    element_t *list = this->first;

    this->first = bsort_rec(list);

    return NULL;
}

static void unique(private_linked_list_t *this)
{
	element_t *current = this->first;
    chain_t *c;
    char *prev_name = NULL;

	while (current)
	{
        c = (chain_t*) current->value;

        if (prev_name == NULL)
            prev_name = c->get_str(c);
        else if (!strcmp(prev_name, c->get_str(c)))
        {
            c->destroy(c);
            remove_element(this, current);
        }
        else
            prev_name = c->get_str(c);

		current = current->next;
	}
}

/**
 * Implementation of linked_list_t.destroy.
 */
static void destroy(private_linked_list_t *this)
{
	void *value;
	/* Remove all list items before destroying list */
	while (remove_first(this, &value) == SUCCESS)
	{
		/* values are not destroyed so memory leaks are possible
		 * if list is not empty when deleting */
	}
	free(this);
}

/**
 * Implementation of linked_list_t.destroy_offset.
 */
static void destroy_offset(private_linked_list_t *this, size_t offset)
{
	element_t *current = this->first, *next;

	while (current)
	{
		void (**method)(void*) = (void (**)(void*)) current->value + offset;
		(*method)(current->value);
		next = current->next;
		free(current);
		current = next;
	}
	free(this);
}

/**
 * Implementation of linked_list_t.destroy_function.
 */
static void destroy_function(private_linked_list_t *this, void (*fn)(void*))
{
	element_t *current = this->first, *next;

	while (current)
	{
		fn(current->value);
		next = current->next;
		free(current);
		current = next;
	}
	free(this);
}

/*
 * Described in header.
 */
linked_list_t *linked_list_create()
{
	private_linked_list_t *this = malloc_thing(private_linked_list_t);

	this->public.get_count = (int (*) (linked_list_t *)) get_count;
	this->public.create_enumerator = (enumerator_t*(*)(linked_list_t*))create_enumerator;
	this->public.get_first = (status_t (*) (linked_list_t *, void **item))get_first;
	this->public.get_last = (status_t (*) (linked_list_t *, void **item))get_last;
	this->public.find_first = (status_t (*) (linked_list_t *, linked_list_match_t,void**,...))find_first;
	this->public.find_last = (status_t (*) (linked_list_t *, linked_list_match_t,void**,...))find_last;
	this->public.insert_first = (void (*) (linked_list_t *, void *item))insert_first;
	this->public.insert_last = (void (*) (linked_list_t *, void *item))insert_last;
    this->public.insert_before = (void (*) (linked_list_t *, enumerator_t *,void *item))insert_before;
	this->public.remove_first = (status_t (*) (linked_list_t *, void **item))remove_first;
	this->public.remove_last = (status_t (*) (linked_list_t *, void **item))remove_last;
	this->public.remove = (int(*)(linked_list_t*, void *item, bool (*compare)(void *,void*)))remove_;
	this->public.remove_at = (void(*)(linked_list_t*, enumerator_t *enumerator))remove_at;
	this->public.invoke_offset = (void (*)(linked_list_t*,size_t,...))invoke_offset;
	this->public.invoke_function = (void (*)(linked_list_t*,linked_list_invoke_t,...))invoke_function;
	this->public.clone_offset = (linked_list_t * (*)(linked_list_t*,size_t))clone_offset;
	this->public.clone_function = (linked_list_t * (*)(linked_list_t*,void*(*)(void*)))clone_function;
    this->public.sort = (linked_list_t* (*)(linked_list_t*))sort;
    this->public.bsort = (linked_list_t* (*)(linked_list_t*))bsort;
    this->public.unique = (linked_list_t* (*)(linked_list_t*))unique;
	this->public.destroy = (void (*) (linked_list_t *))destroy;
	this->public.destroy_offset = (void (*) (linked_list_t *,size_t))destroy_offset;
	this->public.destroy_function = (void (*)(linked_list_t*,void(*)(void*)))destroy_function;

	this->count = 0;
	this->first = NULL;
	this->last = NULL;

	return &this->public;
}
