//
//  Stack.h
//  Stack Implementation
//
//  Created by Richard Zhang on 4/3/14.
//  Copyright (c) 2016 Hear360 All rights reserved.
//

#ifndef __Stack_h_
#define __Stack_h_

template <class T>
class StackNode
{
public:
	T* data;
	StackNode* next;
};

template <class T>
class Stack
{
private:
	StackNode<T>* top;
    
public:
	Stack()
    : top(0)
	{
		
	}
    
	void Push(T* data)
	{
		StackNode<T>* newNode = new StackNode<T>();
		newNode->data = data;
        
		newNode->next = top;
		top = newNode;
	}
    
	StackNode<T>* Top()
	{
		return top;
	}
    
    T* TopData()
    {
        return top->data;
    }
    
	bool Empty()
	{
		return (top == 0) ? true : false;
	}
    
	bool Pop()
	{
		if(top != 0)
		{
			StackNode<T>* tempNode = top;
			top = top->next;
			delete tempNode;
			return true;
		}
		else
		{
			return false;
		}
	}
};

#endif
