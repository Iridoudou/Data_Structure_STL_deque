#ifndef SJTU_DEQUE_HPP
#define SJTU_DEQUE_HPP

#include "exceptions.hpp"
#include<iostream>
#include <cstddef>
#include <cstring>
#include <cmath>


namespace sjtu
{

#define defaultCapacity 383

    template<class T>
    class deque
    {
    public:
        class const_iterator;
        class iterator;
        friend iterator;
        friend const_iterator;
        class node
        {
        public:
            T* data;
            node* prevNode, * nextNode;
            node():data(NULL),prevNode(NULL),nextNode(NULL)
            {}
            node(const node& other, node* p = NULL, node* n = NULL):prevNode(p), nextNode(n)
            {
                if (other.data)
                {
                    data = new T(*(other.data));
                }
                else
                    data = NULL;
            }
            node(const T& d, node* p = NULL, node* n = NULL) :prevNode(p), nextNode(n)
            {
                data = new T(d);
            }
            ~node()
            {
                if(data)
                    delete data;
            }
        };
        class block
        {
        public:
            int size = 0;
            node* head_node, * rear_node;
            block* prevBlock, * nextBlock;
            block(const block& other, block* pre = NULL, block* n = NULL) :prevBlock(pre), nextBlock(n)
            {
                size = other.size;
                head_node = new node;
                rear_node = new node;
                node* tmp = other.head_node->nextNode;
                node* p = head_node;
                while (tmp != other.rear_node)
                {
                    p->nextNode = new node(*tmp, p);
                    p = p->nextNode;
                    tmp = tmp->nextNode;
                }
                p->nextNode = rear_node;
                rear_node->prevNode = p;
            }
            block(const int s = 0, block* p = NULL, block* n = NULL) :size(s),prevBlock(p), nextBlock(n)
            {
                head_node = new node;
                rear_node = new node;
                head_node->nextNode = rear_node;
                rear_node->prevNode = head_node;
            }
            ~block()
            {
                node* tmp = head_node;
                while (tmp != rear_node)
                {
                    node* tmpNode = tmp;
                    tmp = tmp->nextNode;
                    delete tmpNode;
                }
                delete tmp;
            }
        };
        void merge(block* block1, block* block2, iterator& pos)
        {
            block1->nextBlock = block2->nextBlock;
            block2->nextBlock->prevBlock = block1;

            block1->size = block1->size + block2->size;//大小相加
            node* tmp1 = block1->rear_node->prevNode;
            node* tmp2 = block2->head_node->nextNode;
            tmp1->nextNode = tmp2;
            tmp2->prevNode = tmp1;
            block1->rear_node->prevNode = block2->rear_node->prevNode;
            block2->rear_node->prevNode->nextNode = block1->rear_node;
            //链接block2的头尾
            block2->head_node->nextNode = block2->rear_node;
            block2->rear_node->prevNode = block2->head_node;
            pos.currentBlock = block1;
            delete block2;
        }
        void split(block* blockToSplit, iterator& pos)
        {
            bool flag = false;
            int halfSize = (blockToSplit->size) / 2;//后一半的size
            //新block
            block* rightHalf = new block;
            rightHalf->size = halfSize;
            rightHalf->prevBlock = blockToSplit;
            rightHalf->nextBlock = blockToSplit->nextBlock;
            blockToSplit->nextBlock->prevBlock = rightHalf;
            blockToSplit->nextBlock = rightHalf;

            //重新指
            node* tmpRear = blockToSplit->rear_node->prevNode;
            node* tmpHead = tmpRear;
            for (int i = 1; i < halfSize; i++)
            {
                if (pos.currentNode == tmpHead)//pos指向的位置被移动到后一个block了
                    flag = true;
                tmpHead = tmpHead->prevNode;
            }
            if (pos.currentNode == tmpHead)//pos指向的位置被移动到后一个block了
                flag = true;

            node* pre = tmpHead->prevNode;
            tmpRear->nextNode = rightHalf->rear_node;
            rightHalf->rear_node->prevNode = tmpRear;

            tmpHead->prevNode = rightHalf->head_node;
            rightHalf->head_node->nextNode = tmpHead;
            //绕过这些结点
            blockToSplit->rear_node->prevNode = pre;
            pre->nextNode = blockToSplit->rear_node;

            blockToSplit->size = blockToSplit->size - halfSize;

            if (flag && pos.currentBlock != NULL)
                pos.currentBlock = rightHalf;
        }
        block* head_deque;
        block* rear_deque;
        int maxSize = 0;
    public:
        class iterator
        {
        public:
            deque<T>* currentDeque;
            node* currentNode;
            block* currentBlock;
        public:
            //构造函数
            iterator(deque<T>* d = NULL, node* n = NULL, block* b = NULL) :currentDeque(d), currentNode(n), currentBlock(b)
            {}
            iterator(const iterator& other) :currentDeque(other.currentDeque), currentNode(other.currentNode), currentBlock(other.currentBlock)
            {}
            iterator(const const_iterator& other) :currentDeque(other.currentDeque), currentNode(other.currentNode), currentBlock(other.currentBlock)
            {}
            //返回指向位置
            int getPos() const
            {
                node* tmpNode = currentNode;
                block* tmpBlock = currentBlock;
                int cnt = -1;
                while (tmpNode != tmpBlock->head_node)
                {
                    cnt++;
                    tmpNode = tmpNode->prevNode;
                }
                tmpBlock = tmpBlock->prevBlock;
                while (tmpBlock != currentDeque->head_deque)
                {
                    cnt += tmpBlock->size;
                    tmpBlock = tmpBlock->prevBlock;
                }
                return cnt;
            }
            //itr+n
            iterator operator+(const int& n) const
            {
                if (n < 0)
                    return *this - (-1 * n);
                if (n == 0)
                    return *this;
                iterator tmpItr(*this);
                tmpItr += n;
                return tmpItr;
            }
            //itr-n
            iterator operator-(const int& n) const
            {
                if (n < 0)
                    return (*this) + (-1 * n);
                else
                {
                    iterator tmpItr(*this);
                    tmpItr -= n;
                    return tmpItr;
                }
            }
            //两个itr之间的距离
            int operator-(const iterator& rhs) const
            {
                if (this->currentDeque != rhs.currentDeque)
                    throw invalid_iterator();
                int pos1 = rhs.getPos();
                int pos2 = getPos();
                return pos2 - pos1;
            }
            //itr+=n
            iterator& operator+=(const int& n)
            {
                if (n == 0)
                    return *this;
                if (n < 0)
                {
                    (*this) -= (-1 * n);
                    return *this;
                }
                if (*this == currentDeque->end())
                    throw index_out_of_bound();
                int tmp = n;
                while (currentNode->nextNode != currentBlock->rear_node)
                {
                    currentNode = currentNode->nextNode;
                    tmp--;
                    if (tmp == 0)
                        return (*this);
                }
                while (currentBlock->nextBlock != currentDeque->rear_deque)
                {
                    currentBlock = currentBlock->nextBlock;
                    if (currentBlock->size < tmp)//跳到下一个block的头
                    {
                        tmp -= currentBlock->size;
                    }
                    else//在当前block
                    {
                        currentNode = currentBlock->head_node;
                        while (tmp > 0)
                        {
                            currentNode = currentNode->nextNode;
                            tmp--;
                        }
                        return *this;
                    }
                }
                if (tmp == 1)
                {
                    *this = currentDeque->end();
                    return *this;
                }

                throw index_out_of_bound();
            }
            //itr-=n
            iterator& operator-=(const int& n)
            {
                if (n == 0)
                    return *this;
                else if (n < 0)
                {
                    *this += (-n);
                    return *this;
                }
                else
                {
                    if (*this == currentDeque->begin())
                        throw index_out_of_bound();
                    int tmp = n;
                    //std::cout << tmp<<"hhh\n";
                    while (currentNode->prevNode != currentBlock->head_node)
                    {
                        currentNode = currentNode->prevNode;
                        tmp--;
                        if (tmp == 0)
                            return *this;
                    }
                    while (currentBlock->prevBlock != currentDeque->head_deque)
                    {
                        currentBlock = currentBlock->prevBlock;
                        if (currentBlock->size >= tmp)
                        {
                            currentNode = currentBlock->rear_node;
                            for (int i = 0; i < tmp; i++)
                            {
                                currentNode = currentNode->prevNode;

                            }
                            return (*this);
                        }
                        else//在当前block
                        {
                            tmp -= currentBlock->size;
                        }
                    }
                    throw invalid_iterator();
                }
            }
            //itr++
            iterator operator++(int)
            {
                iterator tmp(*this);
                ++* this;
                return tmp;
            }
            //++itr
            iterator& operator++()
            {
                if (currentNode != currentBlock->rear_node->prevNode)//不是最后一个结点
                {
                    currentNode = currentNode->nextNode;
                    return *this;
                }
                else//是最后一个结点
                {
                    if (currentBlock->nextBlock == currentDeque->rear_deque)
                    {
                        currentNode = currentNode->nextNode;
                    }
                    else
                    {
                        currentBlock = currentBlock->nextBlock;
                        currentNode = currentBlock->head_node->nextNode;
                    }
                    return *this;
                }
            }
            //itr--
            iterator operator--(int)
            {
                iterator tmp(*this);
                --* this;
                return tmp;
            }
            //--itr
            iterator& operator--()
            {
                if (currentNode != currentBlock->head_node->nextNode)//不是第一个结点
                {
                    currentNode = currentNode->prevNode;
                    return *this;
                }
                else//是第一个结点
                {
                    if (currentBlock->prevBlock == currentDeque->head_deque)
                    {
                        currentNode = currentNode->prevNode;
                    }
                    else
                    {
                        currentBlock = currentBlock->prevBlock;
                        currentNode = currentBlock->rear_node->prevNode;
                    }
                    return *this;
                }
            }
            //*itr
            T& operator*() const
            {
                if (currentNode == NULL || currentBlock == currentDeque->head_deque || currentBlock == currentDeque->rear_deque || currentNode == currentBlock->head_node || currentNode == currentBlock->rear_node)
                    throw invalid_iterator();
                return *(currentNode->data);
            }
            //->重载
            T* operator->() const noexcept
            {
                if (currentBlock == currentDeque->head_deque || currentBlock == currentDeque->rear_deque || currentNode == currentBlock->head_node || currentNode == currentBlock->rear_node)
                    throw invalid_iterator();
                return currentNode->data;
            }
            //==重载
            bool operator==(const iterator& rhs) const
            {
                return currentNode == rhs.currentNode;
            }
            bool operator==(const const_iterator& rhs) const
            {
                return currentNode == rhs.currentNode;
            }
            //!=重载
            bool operator!=(const iterator& rhs) const
            {
                return !(currentNode == rhs.currentNode);
            }
            bool operator!=(const const_iterator& rhs) const
            {
                return !(currentNode == rhs.currentNode);
            }

        };
        class const_iterator
        {
        public:
            const deque<T>* currentDeque;
            node* currentNode;
            block* currentBlock;
        public:
            //构造函数
            const_iterator(const deque<T>* d = NULL, node* n = NULL, block* b = NULL) :currentDeque(d), currentNode(n), currentBlock(b)
            {}
            const_iterator(const const_iterator& other) :currentDeque(other.currentDeque), currentNode(other.currentNode), currentBlock(other.currentBlock)
            {}
            const_iterator(const iterator& other) :currentDeque(other.currentDeque), currentNode(other.currentNode), currentBlock(other.currentBlock)
            {}
            //返回指向位置
            int getPos()const
            {
                node* tmpNode = currentNode;
                block* tmpBlock = currentBlock;
                int cnt = -1;
                while (tmpNode != tmpBlock->head_node)
                {
                    cnt++;
                    tmpNode = tmpNode->prevNode;
                }
                tmpBlock = tmpBlock->prevBlock;
                while (tmpBlock != currentDeque->head_deque)
                {
                    cnt += tmpBlock->size;
                    tmpBlock = tmpBlock->prevBlock;
                }
                return cnt;
            }
            //itr+n
            const_iterator operator+(const int& n) const
            {
                if (n < 0)
                    return *this - (-1 * n);
                if (n == 0)
                    return *this;
                const_iterator tmpItr(*this);
                tmpItr += n;
                return tmpItr;
            }
            //itr-n
            const_iterator operator-(const int& n) const
            {
                if (n < 0)
                    return *this + (-1 * n);
                if (n == 0)
                    return *this;
                const_iterator tmpItr(*this);
                tmpItr -= n;
                return tmpItr;
            }
            int operator-(const const_iterator& rhs) const
            {
                if (this->currentDeque != rhs.currentDeque)
                    throw invalid_iterator();
                int pos1 = rhs.getPos();
                int pos2 = getPos();
                return pos2 - pos1;
            }
            //itr+=n
            const_iterator& operator+=(const int& n)
            {
                if (n == 0)
                    return *this;
                if (n < 0)
                {
                    *this -= (-1 * n);
                    return *this;
                }
                if (*this == currentDeque->cend())
                    throw index_out_of_bound();
                int tmp = n;
                while (currentNode->nextNode != currentBlock->rear_node)
                {
                    currentNode = currentNode->nextNode;
                    tmp--;
                    if (tmp == 0)
                        return *this;
                }
                while (currentBlock->nextBlock != currentDeque->rear_deque)
                {
                    currentBlock = currentBlock->nextBlock;
                    if (currentBlock->size < tmp)//跳到下一个block的头
                    {
                        tmp -= currentBlock->size;
                    }
                    else//在当前block
                    {
                        currentNode = currentBlock->head_node;
                        while (tmp)
                        {
                            currentNode = currentNode->nextNode;
                            tmp--;
                        }
                        return *this;
                    }
                }
                if (tmp == 1)
                {
                    *this = currentDeque->cend();
                    return *this;
                }

                throw index_out_of_bound();
            }
            //itr-=n
            const_iterator& operator-=(const int& n)
            {
                if (n == 0)
                    return *this;
                else if (n < 0)
                {
                    *this += (-n);
                    return *this;
                }
                else
                {
                    if (*this == currentDeque->cbegin())
                        throw index_out_of_bound();
                    int tmp = n;
                    //std::cout << tmp<<"hhh\n";
                    while (currentNode->prevNode != currentBlock->head_node)
                    {
                        currentNode = currentNode->prevNode;
                        tmp--;
                        if (tmp == 0)
                            return *this;
                    }
                    while (currentBlock->prevBlock != currentDeque->head_deque)
                    {
                        currentBlock = currentBlock->prevBlock;
                        if (currentBlock->size >= tmp)
                        {
                            currentNode = currentBlock->rear_node;
                            for (int i = 0; i < tmp; i++)
                            {
                                currentNode = currentNode->prevNode;

                            }
                            return (*this);
                        }
                        else//在当前block
                        {
                            tmp -= currentBlock->size;
                        }
                    }
                    throw invalid_iterator();
                }
            }
            //itr++
            const_iterator operator++(int)
            {
                const_iterator tmp(*this);
                ++* this;
                return tmp;
            }
            //++itr
            const_iterator& operator++()
            {
                if (currentNode != currentBlock->rear_node->prevNode)//不是最后一个结点
                {
                    currentNode = currentNode->nextNode;
                    return *this;
                }
                else//是最后一个结点
                {
                    if (currentBlock->nextBlock == currentDeque->rear_deque)
                    {
                        currentNode = currentNode->nextNode;
                    }
                    else
                    {
                        currentBlock = currentBlock->nextBlock;
                        currentNode = currentBlock->head_node->nextNode;
                    }
                    return *this;
                }
            }
            //itr--
            const_iterator operator--(int)
            {
                const_iterator tmp(*this);
                -- *this;
                return tmp;
            }
            //--itr
            const_iterator& operator--()
            {
                if (currentNode != currentBlock->head_node->nextNode)//不是第一个结点
                {
                    currentNode = currentNode->prevNode;
                    return *this;
                }
                else//是第一个结点
                {
                    if (currentBlock->prevBlock == currentDeque->head_deque)
                    {
                        currentNode = currentNode->prevNode;
                    }
                    else
                    {
                        currentBlock = currentBlock->prevBlock;
                        currentNode = currentBlock->rear_node->prevNode;
                    }
                    return *this;
                }
            }
            //*itr
            const T& operator*() const
            {
                if (currentBlock == currentDeque->head_deque || currentBlock == currentDeque->rear_deque || currentNode == currentBlock->head_node || currentNode == currentBlock->rear_node)
                    throw invalid_iterator();
                return *(currentNode->data);
            }
            //->重载
            T* operator->() const noexcept
            {
                if (currentBlock == currentDeque->head_deque || currentBlock == currentDeque->rear_deque || currentNode == currentBlock->head_node || currentNode == currentBlock->rear_node)
                    throw invalid_iterator();
                return currentNode->data;
            }
            //==重载
            bool operator==(const iterator& rhs) const
            {
                return currentNode == rhs.currentNode;
            }
            bool operator==(const const_iterator& rhs) const
            {
                return currentNode == rhs.currentNode;
            }
            //!=重载
            bool operator!=(const iterator& rhs) const
            {
                return !(currentNode == rhs.currentNode);
            }
            bool operator!=(const const_iterator& rhs) const
            {
                return !(currentNode == rhs.currentNode);
            }
        };
        //得到sqrt(maxSize)+Const
        int getSqrtSize()
        {
            return defaultCapacity;
        }




        //构造
        deque()
        {
            head_deque = new block;
            rear_deque = new block;
            block* tmpBlock = new block;
            tmpBlock->prevBlock = head_deque;
            tmpBlock->nextBlock = rear_deque;
            head_deque->nextBlock = tmpBlock;
            rear_deque->prevBlock = tmpBlock;
            maxSize = 0;
        }
        deque(const deque& other)
        {
            head_deque = new block;
            rear_deque = new block;
            block* tmp = other.head_deque->nextBlock;
            block* p = head_deque;
            while (tmp != other.rear_deque)
            {
                p->nextBlock = new block(*tmp);
                p->nextBlock->prevBlock = p;
                p = p->nextBlock;
                tmp = tmp->nextBlock;
            }
            p->nextBlock = rear_deque;
            rear_deque->prevBlock = p;
            maxSize = other.maxSize;
        }
        //析构
        ~deque()
        {
            block* tmp = head_deque;
            while (tmp != rear_deque)
            {
                block* tmpBlock = tmp;
                tmp = tmp->nextBlock;
                delete tmpBlock;
            }
            delete tmp;
        }
        //赋值
        deque& operator=(const deque& other)
        {
            //防止自己复制自己
            if (this == &other)
                return *this;

            //删除
            block* tmp1 = head_deque->nextBlock;
            while (tmp1 != rear_deque)
            {
                block* tmpBlock = tmp1;
                tmp1 = tmp1->nextBlock;
                delete tmpBlock;
            }
            block* tmpCopy = other.head_deque->nextBlock;
            block* tmpPrev = head_deque;
            while (tmpCopy != other.rear_deque)
            {
                tmp1 = new block(*tmpCopy);
                tmp1->prevBlock = tmpPrev;
                tmpPrev->nextBlock = tmp1;
                tmpPrev = tmp1;
                tmpCopy = tmpCopy->nextBlock;
            }
            rear_deque->prevBlock = tmpPrev;
            tmpPrev->nextBlock = rear_deque;
            maxSize = other.maxSize;
            return *this;
        }
        //下标
        T& at(const size_t& pos)
        {
            if (pos < 0 || pos >= maxSize)
                throw index_out_of_bound();
            if (pos <= maxSize / 2)//从前往后遍历
            {
                int cnt = 0;
                block* tmpBlock = head_deque->nextBlock;

                //往后遍历
                while (tmpBlock != rear_deque)
                {
                    if ((cnt + tmpBlock->size) >= pos + 1)//访问这个block
                    {
                        node* tmpNode = tmpBlock->head_node->nextNode;
                        while (tmpNode != tmpBlock->rear_node)
                        {
                            cnt++;

                            if (cnt == pos + 1)
                                return *(tmpNode->data);

                            tmpNode = tmpNode->nextNode;
                        }
                    }
                    else
                    {
                        cnt += tmpBlock->size;
                    }
                    tmpBlock = tmpBlock->nextBlock;
                }
            }
            else//从后往前遍历
            {
                int cnt = 0;
                block* tmpBlock = rear_deque->prevBlock;

                int tmpPos = maxSize - pos - 1;
                //往前遍历
                while (tmpBlock != head_deque)
                {
                    if ((cnt + tmpBlock->size) >= tmpPos + 1)//访问这个block
                    {
                        node* tmpNode = tmpBlock->rear_node->prevNode;
                        while (tmpNode != tmpBlock->head_node)
                        {
                            cnt++;

                            if (cnt == tmpPos + 1)
                                return *(tmpNode->data);

                            tmpNode = tmpNode->prevNode;
                        }
                    }
                    else
                    {
                        cnt += tmpBlock->size;
                    }
                    tmpBlock = tmpBlock->prevBlock;
                }
            }
        }
        const T& at(const size_t& pos) const
        {
            if (pos < 0 || pos >= maxSize)
                throw index_out_of_bound();
            if (pos <= maxSize / 2)//从前往后遍历
            {
                int cnt = 0;
                block* tmpBlock = head_deque->nextBlock;

                //往后遍历
                while (tmpBlock != rear_deque)
                {
                    if ((cnt + tmpBlock->size) >= pos + 1)//访问这个block
                    {
                        node* tmpNode = tmpBlock->head_node->nextNode;
                        while (tmpNode != tmpBlock->rear_node)
                        {
                            cnt++;

                            if (cnt == pos + 1)
                                return *(tmpNode->data);

                            tmpNode = tmpNode->nextNode;
                        }
                    }
                    else
                    {
                        cnt += tmpBlock->size;
                    }
                    tmpBlock = tmpBlock->nextBlock;
                }
            }
            else//从后往前遍历
            {
                int cnt = 0;
                block* tmpBlock = rear_deque->prevBlock;

                int tmpPos = maxSize - pos - 1;
                //往前遍历
                while (tmpBlock != head_deque)
                {
                    if ((cnt + tmpBlock->size) >= tmpPos + 1)//访问这个block
                    {
                        node* tmpNode = tmpBlock->rear_node->prevNode;
                        while (tmpNode != tmpBlock->head_node)
                        {
                            cnt++;

                            if (cnt == tmpPos + 1)
                                return *(tmpNode->data);

                            tmpNode = tmpNode->prevNode;
                        }
                    }
                    else
                    {
                        cnt += tmpBlock->size;
                    }
                    tmpBlock = tmpBlock->prevBlock;
                }
            }
        }
        T& operator[](const size_t& pos)
        {
            if (pos < 0 || pos >= maxSize)
                throw index_out_of_bound();
            if (pos <= maxSize / 2)//从前往后遍历
            {
                int cnt = 0;
                block* tmpBlock = head_deque->nextBlock;

                //往后遍历
                while (tmpBlock != rear_deque)
                {
                    if ((cnt + tmpBlock->size) >= pos + 1)//访问这个block
                    {
                        node* tmpNode = tmpBlock->head_node->nextNode;
                        while (tmpNode != tmpBlock->rear_node)
                        {
                            cnt++;

                            if (cnt == pos + 1)
                                return *(tmpNode->data);

                            tmpNode = tmpNode->nextNode;
                        }
                    }
                    else
                    {
                        cnt += tmpBlock->size;
                    }
                    tmpBlock = tmpBlock->nextBlock;
                }
            }
            else//从后往前遍历
            {
                int cnt = 0;
                block* tmpBlock = rear_deque->prevBlock;

                int tmpPos = maxSize - pos - 1;
                //往前遍历
                while (tmpBlock != head_deque)
                {
                    if ((cnt + tmpBlock->size) >= tmpPos + 1)//访问这个block
                    {
                        node* tmpNode = tmpBlock->rear_node->prevNode;
                        while (tmpNode != tmpBlock->head_node)
                        {
                            cnt++;

                            if (cnt == tmpPos + 1)
                                return *(tmpNode->data);

                            tmpNode = tmpNode->prevNode;
                        }
                    }
                    else
                    {
                        cnt += tmpBlock->size;
                    }
                    tmpBlock = tmpBlock->prevBlock;
                }
            }
        }
        const T& operator[](const size_t& pos) const
        {
            if (pos < 0 || pos >= maxSize)
                throw index_out_of_bound();
            if (pos <= maxSize / 2)//从前往后遍历
            {
                int cnt = 0;
                block* tmpBlock = head_deque->nextBlock;

                //往后遍历
                while (tmpBlock != rear_deque)
                {
                    if ((cnt + tmpBlock->size) >= pos + 1)//访问这个block
                    {
                        node* tmpNode = tmpBlock->head_node->nextNode;
                        while (tmpNode != tmpBlock->rear_node)
                        {
                            cnt++;

                            if (cnt == pos + 1)
                                return *(tmpNode->data);

                            tmpNode = tmpNode->nextNode;
                        }
                    }
                    else
                    {
                        cnt += tmpBlock->size;
                    }
                    tmpBlock = tmpBlock->nextBlock;
                }
            }
            else//从后往前遍历
            {
                int cnt = 0;
                block* tmpBlock = rear_deque->prevBlock;

                int tmpPos = maxSize - pos - 1;
                //往前遍历
                while (tmpBlock != head_deque)
                {
                    if ((cnt + tmpBlock->size) >= tmpPos + 1)//访问这个block
                    {
                        node* tmpNode = tmpBlock->rear_node->prevNode;
                        while (tmpNode != tmpBlock->head_node)
                        {
                            cnt++;

                            if (cnt == tmpPos + 1)
                                return *(tmpNode->data);

                            tmpNode = tmpNode->prevNode;
                        }
                    }
                    else
                    {
                        cnt += tmpBlock->size;
                    }
                    tmpBlock = tmpBlock->prevBlock;
                }
            }
        }
        //第一个元素
        const T& front() const
        {
            if (maxSize == 0)
                throw container_is_empty();
            return *(head_deque->nextBlock->head_node->nextNode->data);
        }
        //最后一个元素
        const T& back() const
        {
            if (maxSize == 0)
                throw container_is_empty();
            return *(rear_deque->prevBlock->rear_node->prevNode->data);
        }
        //返回起始结点
        iterator begin()
        {
            return iterator(this, head_deque->nextBlock->head_node->nextNode, head_deque->nextBlock);
        }
        const_iterator cbegin() const
        {
            return const_iterator(this, head_deque->nextBlock->head_node->nextNode, head_deque->nextBlock);
        }
        //返回末尾结点
        iterator end()
        {
            return iterator(this, rear_deque->prevBlock->rear_node, rear_deque->prevBlock);
        }
        const_iterator cend() const
        {
            return const_iterator(this, rear_deque->prevBlock->rear_node, rear_deque->prevBlock);
        }
        //判空
        bool empty() const
        {
            if (maxSize == 0)
                return true;
            else
                return false;
        }
        //元素个数
        size_t size() const
        {
            return maxSize;
        }
        //清空元素，保留头尾,重构中间的block
        void clear()
        {
            block* tmp = head_deque->nextBlock;
            while (tmp != rear_deque)
            {
                block* tmpBlock = tmp;
                tmp = tmp->nextBlock;
                delete tmpBlock;
            }
            tmp = new block;
            head_deque->nextBlock = tmp;
            rear_deque->prevBlock = tmp;
            tmp->prevBlock = head_deque;
            tmp->nextBlock = rear_deque;
            maxSize = 0;
        }
        //任意位置插入元素
        iterator insert(iterator pos, const T& value)
        {
            if (pos.currentDeque != this)
                throw invalid_iterator();

            node* tmpNode = pos.currentNode;
            node* tmp = new node(value, tmpNode->prevNode, tmpNode);
            tmpNode->prevNode->nextNode = tmp;
            tmpNode->prevNode = tmp;

            pos.currentBlock->size++;
            maxSize++;
            pos--;
            if (pos.currentBlock->size >= (2 * getSqrtSize()))
                split(pos.currentBlock, pos);

            return pos;
        }
        //清除任意位置元素
        iterator erase(iterator pos)
        {
            if (pos.currentDeque != this)
                throw invalid_iterator();
            if (maxSize == 0)
                throw container_is_empty();
            node* tmpNode = pos.currentNode;
            block* tmpBlock = pos.currentBlock;
            pos++;

            tmpNode->prevNode->nextNode = tmpNode->nextNode;
            tmpNode->nextNode->prevNode = tmpNode->prevNode;
            delete tmpNode;
            tmpBlock->size--;
            maxSize--;

            if ((tmpBlock->size + tmpBlock->nextBlock->size) <= getSqrtSize() && tmpBlock->nextBlock != rear_deque)
            {
                merge(tmpBlock, tmpBlock->nextBlock, pos);
                return pos;
            }
            else if (tmpBlock->size == 0 && maxSize != 0)
            {
                pos.currentBlock = tmpBlock->prevBlock;
                pos.currentNode = tmpBlock->prevBlock->rear_node;
                tmpBlock->prevBlock->nextBlock = tmpBlock->nextBlock;
                tmpBlock->nextBlock->prevBlock = tmpBlock->prevBlock;
                delete tmpBlock;
                return pos;
            }
            return pos;
        }
        //在末尾入队
        void push_back(const T& value)
        {
            block* tmpBlock = rear_deque->prevBlock;
            node* tmpNode = tmpBlock->rear_node->prevNode;
            tmpNode->nextNode = new node(value, tmpNode, tmpNode->nextNode);
            tmpBlock->rear_node->prevNode = tmpNode->nextNode;
            tmpBlock->size++;
            maxSize++;
            iterator tmpItr = iterator();
            if (tmpBlock->size >= 2 * getSqrtSize())
                split(tmpBlock, tmpItr);
        }
        //清除末尾元素
        void pop_back()
        {
            if (maxSize == 0)
                throw container_is_empty();
            block* tmpBlock = rear_deque->prevBlock;
            node* tmpNode = tmpBlock->rear_node->prevNode;

            tmpNode->prevNode->nextNode = tmpNode->nextNode;
            tmpNode->nextNode->prevNode = tmpNode->prevNode;
            delete tmpNode;
            tmpBlock->size--;
            maxSize--;
            if (tmpBlock->size == 0 && maxSize != 0)
            {
                tmpBlock->prevBlock->nextBlock = tmpBlock->nextBlock;
                tmpBlock->nextBlock->prevBlock = tmpBlock->prevBlock;
                delete tmpBlock;
            }
            else if (maxSize != 0)
            {
                if (tmpBlock->size >= 2 * getSqrtSize())
                {
                    iterator tmpItr = iterator();
                    split(tmpBlock, tmpItr);
                }
                else if ((tmpBlock->size + tmpBlock->nextBlock->size) <= getSqrtSize() && tmpBlock->nextBlock != rear_deque)
                {
                    iterator tmpItr = iterator();
                    merge(tmpBlock, tmpBlock->nextBlock, tmpItr);
                }
            }
        }
        //在队首入队
        void push_front(const T& value)
        {
            block* tmpBlock = head_deque->nextBlock;
            node* tmpNode = tmpBlock->head_node;

            tmpNode->nextNode = new node(value, tmpNode, tmpNode->nextNode);
            tmpNode->nextNode->nextNode->prevNode = tmpNode->nextNode;
          
            maxSize++;
            tmpBlock->size++;

            if (tmpBlock->size >= 2 * getSqrtSize())
            {
                iterator tmpItr = iterator();
                split(tmpBlock, tmpItr);
            }
        }
        //清除队首元素
        void pop_front()
        {
            if (maxSize == 0)
                throw container_is_empty();
            block* tmpBlock = head_deque->nextBlock;
            node* tmpNode = tmpBlock->head_node->nextNode;
            tmpNode->prevNode->nextNode = tmpNode->nextNode;
            tmpNode->nextNode->prevNode = tmpNode->prevNode;
            delete tmpNode;
            tmpBlock->size--;
            maxSize--;
            if (tmpBlock->size == 0 && maxSize != 0)
            {
                tmpBlock->prevBlock->nextBlock = tmpBlock->nextBlock;
                tmpBlock->nextBlock->prevBlock = tmpBlock->prevBlock;
                delete tmpBlock;
            }
            else if (maxSize != 0)
            {
                if (tmpBlock->size >= 2 * getSqrtSize())
                {
                    iterator tmpItr = iterator();
                    split(tmpBlock, tmpItr);
                }
                else if((tmpBlock->size + tmpBlock->nextBlock->size) <= getSqrtSize() && tmpBlock->nextBlock != rear_deque)
                {
                    iterator tmpItr = iterator();
                    merge(tmpBlock, tmpBlock->nextBlock, tmpItr);
                }
            }
        }
    };
}
#endif