//
//  RingBuffer2.h
//  RingBuffer with adjustable length implemented using Cycled Dlinked List and Stack
//
//  Created by Richard Zhang on 4/2/14.
//  Copyright (c) 2016 Hear360 All rights reserved.
//

#ifndef __CycledDLinkList__
#define __CycledDLinkList__

#include "Stack.hpp"

template <class DT>
class RingBuffer2Node
{
public:
    DT* mData;
    RingBuffer2Node* mNext;
    RingBuffer2Node* mPrev;
};

template <class DT>
class RingBuffer2
{
private:
    RingBuffer2Node<DT>* mHead;
    Stack<DT> mAllNodes;
    //RingBuffer2Node<DT>* mAllNodes;

    int mSize;
    int mTotalSize;
    int mClearSize;

public:
    RingBuffer2(DT* datas, int size, int clearSize = 0)
    : mHead(0)
    , mSize(0)
    , mClearSize(clearSize)
    {
        mTotalSize = size;
        //mAllNodes = new RingBuffer2Node<DT>[size];
        for(int i = 0; i < size; i++)
        {
            mAllNodes.Push(&datas[i]);
            //mAllNodes[i].mData = datas[i];
            //mAllNodes[i].mNext = 0;
            //mAllNodes[i].mPrev = 0;
        }

        InitRing();
        //InitRing(mAllNodes[0]);
    }

    RingBuffer2Node<DT>* GetHead()
    {
        return mHead;
    }

    void InitRing()
    {
        RingBuffer2Node<DT>* node = new RingBuffer2Node<DT>();
        node->mData = mAllNodes.TopData();
        mAllNodes.Pop();

        node->mNext = node;
        node->mPrev = node;
        mHead = node;

        mSize++;
    }

    void AddToRing(RingBuffer2Node<DT>* curNode)
    {
        RingBuffer2Node<DT>* node = new RingBuffer2Node<DT>();
        node->mData = mAllNodes.TopData();
        mAllNodes.Pop();

        RingBuffer2Node<DT>* prevNode = curNode->mPrev;
        prevNode->mNext = node;
        node->mPrev = prevNode;
        curNode->mPrev = node;
        node->mNext = curNode;

        mSize++;
    }

    void RemoveFromRing(RingBuffer2Node<DT>* curNode)
    {
        RingBuffer2Node<DT>* deleteNode = curNode->mPrev;
        RingBuffer2Node<DT>* prevNode = deleteNode->mPrev;
        RingBuffer2Node<DT>* nextNode = deleteNode->mNext;
        prevNode->mNext = nextNode;
        nextNode->mPrev = prevNode;

        mAllNodes.Push(deleteNode->mData);
        //if(mClearSize != 0)
            //memset(&(deleteNode->mData), 0, mClearSize);

        delete deleteNode;

        mSize--;
    }

    void SetEffectiveSize(RingBuffer2Node<DT>* curNode, int size)
    {
        //Increase the size
        if(size > mSize)
        {
            if(size > mTotalSize)
                return;

            int needToAdd = size - mSize;
            for(int i = 0; i < needToAdd; i++)
            {
                AddToRing(curNode);
            }
        }
        //Decrease the size
        else if(size < mSize)
        {
            if(size < 1)
                return;

            int needToDelete = mSize - size;
            for(int i = 0; i < needToDelete; i++)
            {
                RemoveFromRing(curNode);
            }
        }

        //mSize = size;
    }
};

template <class DT>
class RingBuffer2Iterator
{
private:
    RingBuffer2<DT>* mList;
    RingBuffer2Node<DT>* mCurNode;

public:
    RingBuffer2Iterator(RingBuffer2<DT>* list)
    : mList(list)
    , mCurNode(list->GetHead())
	{

	}

	RingBuffer2Node<DT>* GetCurNode()
	{
		return mCurNode;
	}

    DT* GetCurNodeData()
	{
		return mCurNode->mData;
	}

	void Next()
	{
        mCurNode = mCurNode->mNext;
	}

    void Prev()
    {
        mCurNode = mCurNode->mPrev;
    }

    void ReSize(int size)
    {
        mList->SetEffectiveSize(mCurNode, size);
    }
};

#endif /* defined(__BinarualLib__CycledDLinkList__) */
