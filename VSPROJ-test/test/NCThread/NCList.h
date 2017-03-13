/**
 * Copyright @ 2013 - 2015 Suntec Software(Shanghai) Co., Ltd.
 * All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are NOT permitted except as agreed by
 * Suntec Software(Shanghai) Co., Ltd.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 */

/**
 * @file NCList.h
 * @brief
 *
 */

#ifndef NCLIST_H
#define NCLIST_H

#ifndef NCTYPESDEFINE_H
#   include "ncore/NCTypesDefine.h"
#endif

#ifndef __cplusplus
#   error ERROR: This file requires C++ compilation (use a .cpp suffix)
#endif

namespace nutshell
{
    /**
     * @brief a kind of linear sequence container which hold pointers in
     *
     * @class NCList
     *
     * NCList is a kind of sequence container, its elements are ordered following a linear sequence.<br />
     * It keeps a pionter list to your data.
     *
     * @code

    NCList<INT> myList;
    myList<< new INT(1)<< new INT(2)<< new INT(3)<< new INT(4);    //List is 1,2,3,4

    cout<<myList.length();
    // Output:4

    myList.insertPos(new INT(5), 4);    //List is 1,2,3,4,5
    cout<<myList.length();
    // Output:5

    INT* p = myList.replace(new INT(6),0);    //List is 6,2,3,4,5
    delete p;

    NCList<ListItem>::Iterator it = myList.begin();
    do
    {
        cout << *(it.current());
    }
    while(NULL != it());
    // Output: 62345

    it.reset();
    do
    {
        cout << *(it.current());
        delete it.current();    //Delete your elements(pointers)
    }
    while(NULL != it());
    // Output: 62345

    myList.clear();        //Clear the list
     * @endcode
     *
     */
    template <typename T>
    class NCList
    {
    public:
        friend class Iterator;

        /**
         * @brief
         *
         * @class Node
         */
        class Node
        {
        public:
            Node(T* pt):prev(NULL), next(NULL), data(pt)
            {
            }

            Node*   prev;
            Node*   next;
            T*      data;
        private:
            Node()
            {
            }
        };

        /**
         * @brief Iterator for NCList
         *
         * @class Iterator
         * @code
        NCList<INT> list;

        list<< new INT(1)<< new INT(1)<< new INT(3)<< new INT(2)<< new INT(1)
            << new INT(2)<< new INT(1)<< new INT(2)<< new INT(4)<< new INT(4)
            << new INT(3)<< new INT(4)<< new INT(1)<< new INT(3)<< new INT(2);

        NCList<INT>::Iterator it = list.begin();

        //Delete 1
        do
        {
            if (1 == it.current())
            {
                INT* p = it.removeCurrent();
                cout<<"Del:"<<*p<<"__";
                delete p;
            }
//        }
        while(NULL != it());
        cout <<"/tlength:"<<list.length();
        //Output:
        // Del:1__Del:1__Del:1__Del:1__Del:1__      length:10
         * @endcode
         * @author $Author: jjyathefei $
         * @version $Revision: 4985 $
         * @date $Date:: 2012-02-09 17:32:36 #$
         */
        class Iterator
        {
        public:
            Iterator():m_pList(NULL), cur(NULL), fw(NC_FALSE)
            {
            }

            Iterator(NCList *pList, NC_BOOL bFw = NC_TRUE):m_pList(pList), fw(bFw)
            {
                if (fw) {
                    cur = pList->first;
                }
                else {
                    cur = pList->last;
                }
            }

            void set(NCList *pList, NC_BOOL bFw = NC_TRUE)
            {
                m_pList = pList;
                fw = bFw;
                reset();
            }

            /**
             * @brief Check wether an empty iterator
             *
             * @param VOID
             *
             * @return NC_BOOL
             */
            NC_BOOL isEmpty() const
            {
                return (m_pList == NULL);
            }

            /**
             * @brief Get current element
             * @code
            NCList<INT> myList;
            myList<< new INT(1)<< new INT(2)<< new INT(3)<< new INT(4);

            NCList<ListItem>::Iterator it = myList.begin();
            do
            {
                cout << *(it.current());
            }
            while (NULL != it());
            // Output: 1234
             * @endcode
             *
             * @param null
             *
             * @return T*
             */
            inline T* current() const
            {
                if (NULL == cur) {
                    return NULL;
                }
                return cur->data;
            }

            /**
             * @brief Move Iterator(forward or backward)
             * @note Please use do {} while() for iterating
             *
             * @code
            NCList<INT> myList;
            myList<< new INT(1)<< new INT(2)<< new INT(3)<< new INT(4);

            NCList<ListItem>::Iterator it = myList.begin();
            do
            {
                cout << *(it.current());
            } while (NULL != it())

            // Output: 1234
             * @endcode
             *
             * @param null
             *
             * @return T*
             */
            Iterator* operator()()
            {
                if (NULL == m_pList) {
                    return NULL;
                }

                if (NULL != cur) {
                    if (fw) {
                        cur = cur->next;
                    }
                    else {
                        cur = cur->prev;
                    }

                    if (NULL == cur) {
                        return NULL;
                    } // Move to the end
                    else {
                        return this;
                    }
                }
                else {
                    if (fw) {
                        cur = m_pList->first;
                    }
                    else {
                        cur = m_pList->last;
                    }

                    if (NULL == cur) {
                        return NULL;
                    }
                    return this;
                }
            }

            /**
             * @brief reset iterator.
             * @note iterator will move to First if you create at Beginning of list, and move to Last if you create at End of list
             * @param VOID
             * @return VOID
             */
            VOID reset()
            {
                if (NULL == m_pList) {
                    return;
                }

                if (fw) {
                    cur = m_pList->first;
                }
                else {
                    cur = m_pList->last;
                }
            }

            /**
             * @brief remove current element in list.
             * @param VOID
             * @return T* node be removed.
             */
            T* removeCurrent()
            {
                if (NULL == cur) {
                    return NULL;
                }

                Node* tmp = cur;
                if (fw) {
                    cur = tmp->prev;
                }
                else {
                    cur = tmp->next;
                }

                T* data = tmp->data;
                m_pList->deleteNode(tmp);
                return data;
            }

            /**
             * @brief remove current element in list and delete it.
             * @param VOID
             * @return VOID
             */
            VOID deleteCurrent()
            {
                if (NULL == cur) {
                    return;
                }

                Node* tmp = cur;
                if (fw) {
                    cur = tmp->prev;
                }
                else {
                    cur = tmp->next;
                }

                T* data = tmp->data;
                m_pList->deleteNode(tmp);
                delete data;
            }

            /**
             * @brief replace current element in list.
             * @param VOID
             * @return T* node be replaced.
             */
            T* replaceCurrent(T* t)
            {
                if (NULL == cur) {
                    return NULL;
                }
                T* data = cur->data;
                cur->data = t;
                return data;
            }

        private:
            NCList*    m_pList;
            Node*      cur;
            NC_BOOL    fw;    // Forward?
        };

    public:
        NCList():first(NULL), last(NULL), size(0)
        {
        }

        virtual ~NCList()
        {
            clear();
        }

    public:

        /**
         * @code
        NCList<INT> myList;
        if (0 == myList.length())
        {
            //Yeah, it is 0.
        }
         * @endcode
         *
         * @param null
         *
         * @return INT
         */
        /// Get length of current list
        inline INT length() const
        {
            return size;
        }

        /**
         * @code
        NCList<INT> myList;
        if (!myList.HasData())
        {
            //it has no data.
        }
         * @endcode
         *
         * @param null
         *
         * @return NC_BOOL
         */
        /// Does current list have data?
        inline NC_BOOL hasData() const
        {
            return 0 != size;
        }

        /**
         * @note 1.If dst is NULL, Add() is same as Append() <br />2.If "dst" is not in the List, Add() will return NP_FALSE and "t" will not be added into list
         *
         * @code
        NCList<INT> myList;
        INT* p1  = new INT (1);
        INT* p2  = new INT (2);

        myList.Add(p1);
        myList.Add(p2,p1);

        cout<< *myList[0] << *myList[1];
        //Output: 12
         * @endcode
         *
         * @param T*  t
         *
         * @param const T*  dst = NULL
         *
         * @return NC_BOOL
         */
        /// Add pointer "t" after pointer "dst" in the list
        NC_BOOL add(T* t, const T* dst = NULL);

        /**
         * @note 1.If dst is NULL, Add() is same as Push() <br />2.If "dst" is not in the List, Insert() will return NP_FALSE and "t" will not be inserted into list
         *
         * @code
        NCList<INT> myList;
        INT* p1  = new INT (1);
        INT* p2  = new INT (2);

        myList.Insert(p1);
        myList.Insert(p2,p1);

        cout<< *myList[0] << *myList[1];
        //Output: 21
         * @endcode
         *
         * @param T*  t
         *
         * @param const T*  dst = NULL
         *
         * @return NC_BOOL
         */
        /// Insert pointer "t" before poniter "dst" in the list,
        NC_BOOL insert(T* t, const T* dst = NULL);

        /**
         * @note If pos is greater than the length of list, this IF will fail Inserting element and return NP_FALSE.
         *
         * @code
        NCList<INT> myList;
        myList.Insert(new INT(10), 2);    // Failed, 2(pos) > 0(length)
        myList<< new INT(1)<< new INT(2)<< new INT(3)<< new INT(4);
        myList.Insert(new INT(10), 3);    //Succeed 3(pos) < 4(length)
        myList.Insert(new INT(10), 4);    //Succeed 4(pos) = 4(length)
         * @endcode
         *
         * @param T*  t
         *
         * @param INT pos = 0
         *
         * @return NC_BOOL
         */
        /// Insert pointer "t" at pos
        NC_BOOL insertPos(T* t, INT pos = 0);

        /**
         * @note When pos is invalid, Nth(i) will return NULL
         *
         * @code
        NCList<INT> myList;

        INT* a = new INT(1);
        INT* b = new INT(2);
        INT* c = new INT(3);

        myList<<a<<b<<c;

        cout<<*myList.Nth(0)<<*myList.Nth(1)<<*myList.Nth(2);
        //output: 123
        INT * invalidPtr = myList.Nth(6);
        // invalidPtr == NULL
         * @endcode
         *
         * @param INT pos
         *
         * @return T*
         */
        /// Get the element at [pos]
        T* nth(INT pos) const;

        /**
         *@note If List is empty, GetLast() returns NULL
         *
         *@code
        NCList<INT> myList;

        INT* a = new INT(1);
        INT* b = new INT(2);
        INT* c = new INT(3);

        myList<<a<<b<<c;

        cout<<*myList.GetLast();
        //output: 3
         * @endcode
         *
         * @param null
         *
         * @return T*
         */
        /// Get the last element
        T* getLast() const;

        /**
         * @code
        NCList<INT> myList;

        myList.Append(new INT(1));
        myList.Append(new INT(2));

        cout<< *myList[0] << *myList[1];
        //Output: 12
         * @endcode
         *
         * @param T*  t
         *
         * @return VOID
         */
        /// Append pointer "t" at the end of current List
        VOID append(T* t);

        /**
         * @code
        NCList<INT> myList;
        NCList<INT> hisList;

        myList<< new INT(1)<< new INT(2)<< new INT(3)<< new INT(4);
        hisList<< new INT(5)<< new INT(6)<< new INT(7)<< new INT(8);

        hisList.append(myList);

        for (INT i = 0; i < hisList.length(); i++)
        {
            cout << *hisList[i];
        }
        //output: 56781234
         * @endcode
         *
         * @param NCList<T>& t
         *
         * @return VOID
         */
        /// append a List "t" at the end of current List
        VOID append(NCList<T> &t);

        /**
         * @code
        NCList<INT> myList;

        myList.Push(new INT(1));
        myList.Push(new INT(2));

        cout<< *myList[0] << *myList[1];
        //Output: 21
         * @endcode
         *
         * @param T*  t
         *
         * @return VOID
         */
        /// Push pointer "t" at the begining of current List
        VOID push(T* t);

        /**
         * @code
        NCList<INT> myList;
        NCList<INT> hisList;

        myList<< new INT(1)<< new INT(2)<< new INT(3)<< new INT(4);
        hisList<< new INT(5)<< new INT(6)<< new INT(7)<< new INT(8);

        hisList.push(myList);

        for (INT i = 0; i < hisList.length(); i++)
        {
            cout << *hisList[i];
        }
        //output: 12345678
         * @endcode
         *
         * @param NCList<T>& t
         *
         * @return VOID
         */
        /// Push a List at the begining of current List
        VOID push(NCList<T> &t);

        /**
         * @note If pos is invalid, Replace() will fail, and return value is NULL
         *
         * @code
        NCList<INT> myList;
        myList<< new INT(1)<< new INT(2)<< new INT(3)<< new INT(4);
        cout << *myList.Replace(10, 3);    //Succeed 3(pos) < 4(length)
        // output: 4

        cout << *myList.Replace(10, 4);        //Failed,        4(pos) = 4(length), and this will return NULL
        // this will cause Crush(*NULL)
         * @endcode
         *
         * @param T*  t
         *
         * @param INT pos
         *
         * @return T*
         */
        /// Replace the element at [pos] with "t" and return the original one
        T* replace(T* t, INT pos);

        /**
         * @note If list is empty, Pop() returns NULL
         *
         * @code
        NCList<INT> myList;
        myList<< new INT(1)<< new INT(2)<< new INT(3)<< new INT(4);
        cout << *myList.pop();
        // output: 1

        cout << myList.length();
        // output:3
         * @endcode
         *
         * @param null
         *
         * @return T*
         */
        /// Pop first element away from list
        T* pop();

        /**
         * @note if pos is invalid, TakeAt() fails and returns NULL
         *
         * @code
        NCList<INT> myList;
        myList<< new INT(1)<< new INT(2)<< new INT(3)<< new INT(4);
        cout << *myList.takeAt(3);
        // output: 4

        cout << myList.length();
        // output:3
         * @endcode
         *
         * @param INT pos
         *
         * @return T*
         */
        /// Take the element away at pos from list(element will be deleted from list)
        T* takeAt(INT pos);

        /**
         * @note if pos is invalid, TakeAt() fails and returns NULL
         *
         * @code
        NCList<INT> myList;
        INT* p = new INT(1);
        myList<< p;
        delete (myList.take(p));

        cout << myList.length();
        // output:0
         * @endcode
         *
         * @param const T* t
         * @param NC_BOOL fw = NP_TRUE fw:NP_TRUE, search forward, NP_FALSE, search backward
         *
         * @return T*
         */
        /// Take the specfied element away from list(element will be deleted from list)
        T* take(const T* t, NC_BOOL fw = NC_TRUE);

        /**
         * @note If list is empty, TakeFirst() returns NULL
         *
         * @code
        NCList<INT> myList;
        myList<<new INT(1)<< new INT(2)<< new INT(3)<< new INT(4);
        cout << *myList.takeFirst();
        // output: 1

        cout << myList.length();
        // output:3
         * @endcode
         *
         * @param null
         *
         * @return T*
         */
        /// take first element away from list
        T* takeFirst();

        /**
         * @note If list is empty, TakeLast() returns NULL
         *
         * @code
        NCList<INT> myList;
        myList<<new INT(1)<< new INT(2)<< new INT(3)<< new INT(4);
        cout << *myList.takeLast();
        // output: 4

        cout << myList.length();
        // output:3
         * @endcode
         *
         * @param null
         *
         * @return T*
         */
        /// take last element away from list
        T* takeLast();

        /**
         * @note Clear() only delete all the node saved in the list, but don't delete your elements
         *
         * @code
        NCList<INT> myList;
        myList<< new INT(1)<< new INT(2)<< new INT(3)<< new INT(4);
        for (INT i = 0; i < myList.length(); i++)
        {
            delete myList[i];    // you have to delete your elements yourself
        }
        myList.clear();

        cout << myList.length();
        // output:0
         * @endcode
         *
         * @param null
         *
         * @return VOID
         */
        /// Clear the list
        VOID clear();

        /**
         * @note ClearData() only delete all the node saved in the list, and delete your elements
         *
         * @code
        NCList<INT> myList;
        myList<< new INT(1)<< new INT(2)<< new INT(3)<< new INT(4);
        myList.ClearData();

        cout << myList.length();
        // output:0
         * @endcode
         *
         * @param null
         *
         * @return VOID
         */
        /// Clear the list and delete all elements
        VOID clearData();

        // Iterator

        /**
         * @note Begin() will set the moving direction of Iterator: Forward<br/>Please don't create an Iterator in an empty list
         *
         * @code
        NCList<INT> myList;
        myList<< new INT(1)<< new INT(2)<< new INT(3)<< new INT(4);
        NCList<INT>::Iterator it = myList.begin();
        do
        {
            cout<<*(it.current());
        }
        while(NULL != it())
        //Output:1234
         * @endcode
         *
         * @param null
         *
         * @return Iterator
         */
        /// Get Iterator witch pointed to the beginning of the list
        Iterator begin()
        {
            return Iterator(this);
        }

        /**
         *     @note End() will set the moving direction of Iterator: backward<br/>Please don't create an Iterator in an empty list
         *
         * @code
        NCList<INT> myList;
        myList<< new INT(1)<< new INT(2)<< new INT(3)<< new INT(4);

        NCList<INT>::Iterator it = myList.end();
        do
        {
            cout<<*(it.current());
        }
        while(NULL != it())
        //Output:4321
         * @endcode
         *
         * @param null
         *
         * @return Iterator
         */
        /// Get Iterator witch pointed to the end of the list
        Iterator end()
        {
            return Iterator(this, NC_FALSE);
        }

        /**
         * @note If fw=NP_TRUE, the Iterator of the first same data with the specified value in list will be returned<br/>If dw=NP_FALSE, the last's Iterator will be returned<br/>An empty iterator will be returned if the value cannot be found
         *
         * @code
        NCList<INT> myList;
        INT *p1 = new INT(1);
        INT *p2 = new INT(2);
        INT *p3 = new INT(3);
        INT *p3 = new INT(4);
        myList << p1 << p2 << p3 << p2 << p4;

        NCList<INT>::Iterator it = myList.find(p2);
        if (!it.isEmpty())
        {
            it.removeCurrent();
        }
        //Output:4231
         * @endcode
         *
         * @param const T* t
         * @param NC_BOOL fw = NP_TRUE fw:NP_TRUE, search forward, NP_FALSE, search backward
         *
         * @return Iterator
         */
        /// Get Iterator witch pointed to the specified value
        Iterator find(const T* t, NC_BOOL fw = NC_TRUE);

    public:

        /**
         * @code
        NCList<INT> myList;
        myList<< new INT(1)<< new INT(2)<< new INT(3)<< new INT(4);

        NCList<INT> hisList(myList);

        myList.clear();
        cout << hisList.length();
        // Output: 4
         * @endcode
         *
         * @param NCList<T>& list
         *
         * @return NCList<T>&
         */
        /// evaluate the old List to a new list
        NCList(const NCList<T> &list);

        /**
         * @code
        NCList<INT> myList;
        myList<< new INT(1)<< new INT(2)<< new INT(3)<< new INT(4);

        NCList<INT> hisList;
        hisList = myList;

        myList.clear();
        cout << hisList.length();
        // Output: 4
         * @endcode
         *
         * @param NCList<T>& list
         *
         * @return NCList<T>&
         */
        /// evaluate the old List to a new list
        NCList<T>& operator= (const NCList<T> &list);

        /**
         * @code
        NCList<INT> myList;
        myList<< new INT(1)<< new INT(2)<< new INT(3)<< new INT(4);

        cout << *myList[0] << *myList[1] << *myList[2] << *myList[3];
        // Output: 1234
         * @endcode
         *
         * @param INT pos
         *
         * @return T*
         */
        /// Get element operator, same as NCList<T>::Nth()
        inline T* operator[](INT pos) const
        {
            return nth(pos);
        }

        /**
         * @code
        NCList<INT> myList;
        myList<< new INT(1)<< new INT(2)<< new INT(3)<< new INT(4);

        cout << *myList[0] << *myList[1] << *myList[2] << *myList[3];
        // Output: 1234
         * @endcode
         *
         * @param T*  t
         *
         * @return NCList<T>&
         */
        /// Append element operator, same as NCList<T>::Append(T* t)
        inline NCList<T>& operator<< (T* t)
        {
            append(t);
            return *this;
        }

        /**
         * @code
        NCList<INT> myList;
        myList<< new INT(1)<< new INT(2)<< new INT(3)<< new INT(4);

        NCList<INT> hisList;
        hisList<<myList;

        cout << *hisList[0] << *hisList[1] << *hisList[2] << *hisList[3];
        // Output: 1234
         * @endcode
         *
         * @param NCList<T>&  t
         *
         * @return NCList<T>&
         */
        /// Append list operator, same as NCList<T>::append(const NCList<T> &t)
        inline NCList<T>& operator<< (NCList<T>& t)
        {
            append(t);
            return *this;
        }

    protected:
        VOID deleteNode(Node* pNode);

    protected:
        Node*   first;
        Node*   last;
        INT         size;
    };

    /********************************************************************
        Implement...

     ********************************************************************/

    template <typename T>
    T* NCList<T>::nth(INT pos) const
    {
        if (pos >= size || 0 > pos) {
            return NULL;
        }

        Node* pNode = first;
        for (INT i = 0; i < pos; i++) {
            pNode = pNode->next;
        }
        return pNode->data;
    }

    template <typename T>
    T* NCList<T>::getLast() const
    {
        if (NULL == last) {
            return NULL;
        }

        return last->data;
    }

    template <typename T>
    NC_BOOL NCList<T>::add(T* t, const T* dst)
    {
        if (NULL == dst) {
            append(t);
            return NC_TRUE;
        }

        NC_BOOL bFind = NC_FALSE;
        Node* pNode = first;
        while (NULL != pNode) {
            if (dst == pNode->data) {
                bFind = NC_TRUE;
                break;
            }
            else {
                pNode = pNode->next;
            }
        }

        if (bFind) {
            if (last == pNode) {
                append(t);
            }
            else {
                Node* insertNode = new Node(t);
                pNode->next->prev = insertNode;
                insertNode->next = pNode->next;
                insertNode->prev = pNode;
                pNode->next = insertNode;
                size++;
            }
        }

        return bFind;
    }

    template <typename T>
    NC_BOOL NCList<T>::insert(T* t, const T* dst)
    {
        if (NULL == dst) {
            push(t);
            return NC_TRUE;
        }

        NC_BOOL bFind = NC_FALSE;
        Node* pNode = first;
        while (NULL != pNode) {
            if (dst == pNode->data) {
                bFind = NC_TRUE;
                break;
            }
            else {
                pNode = pNode->next;
            }
        }

        if (bFind) {
            if (first == pNode) {
                push(t);
            }
            else {
                Node* insertNode = new Node(t);
                pNode->prev->next = insertNode;
                insertNode->prev = pNode->prev;
                insertNode->next = pNode;
                pNode->prev = insertNode;
                size++;
            }
        }

        return bFind;
    }

    template <typename T>
    VOID NCList<T>::append(T* t)
    {
        Node* pNode = new Node(t);

        if (0 == size) {
            first = pNode;
            last = pNode;
        }
        else {
            last->next = pNode;
            pNode->prev = last;
            last = pNode;
        }

        size++;
    }

    template <typename T>
    VOID NCList<T>::append(NCList<T> &t)
    {
        for (INT i = 0; i < t.length(); i++) {
            append(t[i]);
        }
    }

    template <typename T>
    VOID NCList<T>::push(T* t)
    {
        Node* pNode = new Node(t);

        if (0 == size) {
            first = pNode;
            last = pNode;
        }
        else {
            first->prev = pNode;
            pNode->next = first;
            first = pNode;
        }

        size++;
    }

    template <typename T>
    VOID NCList<T>::push(NCList<T> &t)
    {
        INT total = t.length();
        for (INT i=1; i <= total; i++) {
            push(t[total - i]);
        }
    }

    template <typename T>
    NC_BOOL NCList<T>::insertPos(T* t, INT pos)
    {
        // Pos is invalid, drop it.
        if (pos > size || 0 > pos) {
            return NC_FALSE;
        }

        if (0 == pos) {
            push(t);
        }
        else if (pos == size) {
            append(t);
        }
        else {
            Node* pNode = first;
            for (INT i = 0; i < pos; i++) {
                pNode = pNode->next;
            }

            Node* pInsertNode = new Node(t);

            pInsertNode->next = pNode;
            pInsertNode->prev = pNode->prev;
            pNode->prev->next = pInsertNode;
            pNode->prev = pInsertNode;

            size++;
        }

        return NC_TRUE;
    }

    template <typename T>
    T* NCList<T>::replace(T* t, INT pos)
    {
        if (pos >= size || 0 > pos) {
            return NULL;
        }

        Node* pNode = first;
        for (INT i = 0; i < pos; i++) {
            pNode = pNode->next;
        }

        T* rtn = pNode->data;
        pNode->data = t;

        return rtn;
    }

    template <typename T>
    T* NCList<T>::pop()
    {
        return takeFirst();
    }

    template <typename T>
    T* NCList<T>::takeAt(INT pos)
    {
        // pos invalid, take the first one
        if (pos >= size || 0 > pos) {
            return NULL;
        }
        else if (0 == pos) {
            return takeFirst();
        }
        else if (pos == size - 1) {
            return takeLast();
        }
        else {
            Node* pNode = first;
            for (INT i = 0; i < pos; i++) {
                pNode = pNode->next;
            }

            T* data = pNode->data;
            deleteNode(pNode);
            return data;
        }
    }

    template <typename T>
    T* NCList<T>::take(const T* t, NC_BOOL fw)
    {
        Node* pNode = last;
        if (fw) {
            pNode = first;
        }

        while (NULL != pNode) {
            if (pNode->data == t) {
                T* data = pNode->data;
                deleteNode(pNode);
                return data;
            }

            if (fw) {
                pNode = pNode->next;
            }
            else {
                pNode = pNode->prev;
            }
        }
        return NULL;
    }

    template <typename T>
    T* NCList<T>::takeFirst()
    {
        if (0 == size) {
            return NULL;
        }

        T* t = first->data;
        deleteNode(first);
        return t;
    }

    template <typename T>
    T* NCList<T>::takeLast()
    {
        if (0 == size) {
            return NULL;
        }

        T* t = last->data;
        deleteNode(last);
        return t;
    }

    template <typename T>
    VOID NCList<T>::clear()
    {
        Node* pNode = first;
        while (NULL != pNode) {
            Node* tmp = pNode;
            pNode = pNode->next;
            delete tmp;
        }

        first = NULL;
        last = NULL;
        size = 0;
    }

    template <typename T>
    VOID NCList<T>::clearData()
    {
        Node* pNode = first;
        while (NULL != pNode) {
            Node* tmp = pNode;
            pNode = pNode->next;
            delete tmp->data;
            delete tmp;
        }

        first = NULL;
        last = NULL;
        size = 0;
    }

    template <typename T>
    typename NCList<T>::Iterator NCList<T>::find(const T* t, NC_BOOL fw)
    {
        if (0 == size) {
            return Iterator();
        }

        Iterator it(this, fw);

        do {
            if (it.current() == t) {
                return it;
            }
        } while (NULL != it());

        return Iterator();
    }

    template <typename T>
    VOID NCList<T>::deleteNode(Node* pNode)
    {
        if (0 == size || NULL == pNode) {
            return;
        }

        if (pNode == first) {
            if (1 == size) {
                first = NULL;
                last = NULL;
                delete pNode;
            }
            else {
                first = pNode->next;
                first->prev = NULL;
                delete pNode;
            }
        }
        else if (pNode == last) {
            if (1 == size) {
                first = NULL;
                last = NULL;
                delete pNode;
            }
            else {
                last = pNode->prev;
                last->next = NULL;
                delete pNode;
            }
        }
        else {
            pNode->prev->next = pNode->next;
            pNode->next->prev = pNode->prev;
            delete pNode;
        }

        size--;
    }

    template <typename T>
    NCList<T>::NCList(const NCList<T> &list)
    :first(NULL), last(NULL), size(0)
     {
        for (INT i = 0; i < list.length(); i++) {
            append(list[i]);
        }
     }

    template <typename T>
    NCList<T>& NCList<T>::operator= (const NCList<T> &list)
    {
        if (this == &list) {
                return *this;
        }

        for (INT i = 0; i < list.length(); i++) {
            append(list[i]);
        }

        return *this;
    }

    /**
     * @brief
     *
     * @class NCObjList
     */
    template <typename T>
    class NCObjList : public NCList<T>
    {
    public:

        virtual ~NCObjList()
        {
            NCList<T>::clearData();
        }

        T& append();

        T& push();
    };

    template <typename T>
    T& NCObjList<T>::append()
    {
        T* t = new T();
        NCList<T>::append(t);
        return *t;
    }

    template <typename T>
    T& NCObjList<T>::push()
    {
        T* t = new T();
        NCList<T>::push(t);
        return *t;
    }

}

#endif /* NCLIST_H */
/* EOF */
