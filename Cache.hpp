#ifndef Cache_hpp
#define Cache_hpp

#include <map>
#include <iostream>

namespace ECE141
{
    template <typename KeyType, typename ValueType>
    struct CacheNode
    {
        CacheNode(const KeyType key, const ValueType value) : key(key), value(value) {}
        KeyType key;
        ValueType value;
        CacheNode *next;
        CacheNode *prev;
    };

    template <typename KeyType, typename ValueType>
    struct FreqNode
    {
        int freq;
        int size;
        FreqNode *next;
        FreqNode *prev;
        CacheNode<KeyType, ValueType> *head;
        CacheNode<KeyType, ValueType> *tail;

        FreqNode(int freq, const KeyType defaultKey, const ValueType defaultValue) : freq(freq), size(0)
        {
            head = new CacheNode<KeyType, ValueType>(defaultKey, defaultValue);
            tail = new CacheNode<KeyType, ValueType>(defaultKey, defaultValue);
            head->next = tail;
            head->prev = nullptr;
            tail->prev = head;
            tail->next = nullptr;
        }
    };

    template <typename KeyType, typename ValueType>
    struct DoubleLinkedList
    {
        DoubleLinkedList(int capacity, const KeyType defaultKey, const ValueType defaultValue) : capacity(capacity), size(0)
        {
            head = new CacheNode<KeyType, ValueType>(defaultKey, defaultValue);
            tail = new CacheNode<KeyType, ValueType>(defaultKey, defaultValue);
            head->next = tail;
            head->prev = nullptr;
            tail->prev = head;
            tail->next = nullptr;
        }

        void addFirst(CacheNode<KeyType, ValueType> *node, bool move = false)
        {
            if (size == capacity && !move)
            {
                removeLast();
            }
            CacheNode<KeyType, ValueType> *nxt = head->next;
            head->next = node;
            node->prev = head;
            node->next = nxt;
            nxt->prev = node;
            if (move)
                return;
            size++;
        }

        void removeLast()
        {
            if (size == 0)
                return;
            CacheNode<KeyType, ValueType> *toRemove = tail->prev, *pre = toRemove->prev;
            tail->prev = pre;
            pre->next = tail;
            size--;
            delete toRemove;
        }

        CacheNode<KeyType, ValueType> *head;
        CacheNode<KeyType, ValueType> *tail;
        int capacity;
        int size;
    };

    template <typename KeyType, typename ValueType>
    class Cache
    {
    public:
        Cache(){};
        virtual bool contains(const KeyType key) = 0;
        virtual ValueType get(const KeyType key) = 0;
        virtual void put(const KeyType key, const ValueType value) = 0;
    };

    template <typename KeyType, typename ValueType>
    class LFUCache : public Cache<KeyType, ValueType>
    {
    private:
        std::map<KeyType, FreqNode<KeyType, ValueType> *> nodeMap;
        FreqNode<KeyType, ValueType> *head;
        int capacity;
        KeyType defaultKey;
        ValueType defaultValue;

    public:
        LFUCache(int capacity, const KeyType defaultKey, const ValueType defaultValue) : capacity(capacity), defaultKey(defaultKey), defaultValue(defaultValue)
        {
            head = new FreqNode<KeyType, ValueType>(0, defaultKey, defaultValue);
            head->prev = nullptr;
            head->next = nullptr;
        }
        ~LFUCache() { delete head; }

        void insertNode(CacheNode<KeyType, ValueType> *cur, FreqNode<KeyType, ValueType> *freqNode, const KeyType key)
        {
            freqNode->tail->prev->next = cur;
            cur->prev = freqNode->tail->prev;
            cur->next = freqNode->tail;
            freqNode->tail->prev = cur;
            nodeMap[key] = freqNode;
            freqNode->size++;
        }

        void moveNode(CacheNode<KeyType, ValueType> *cur, FreqNode<KeyType, ValueType> *curFreq, const KeyType key)
        {
            cur->prev->next = cur->next;
            cur->next->prev = cur->prev;
            int newFreq = curFreq->freq + 1;
            if (curFreq->next != nullptr && (curFreq->next->freq) == newFreq)
            {
                insertNode(cur, curFreq->next, key);
            }
            else
            {
                FreqNode<KeyType, ValueType> *newFreqNode = new FreqNode<KeyType, ValueType>(newFreq, defaultKey, defaultValue);
                newFreqNode->next = curFreq->next;
                if (curFreq->next != nullptr)
                    curFreq->next->prev = newFreqNode;
                curFreq->next = newFreqNode;
                newFreqNode->prev = curFreq;

                insertNode(cur, newFreqNode, key);
            }
            curFreq->size--;
        }

        virtual bool contains(const KeyType key)
        {
            if (capacity == 0)
                return false;
            if (nodeMap.find(key) == nodeMap.end())
                return false;
            return true;
        }

        virtual ValueType get(const KeyType key)
        {
            if (!contains(key))
                return defaultValue;

            auto iter = nodeMap.find(key);
            FreqNode<KeyType, ValueType> *curFreq = iter->second;
            CacheNode<KeyType, ValueType> *cur = curFreq->head;
            while (cur != nullptr)
            {
                if (cur->key == key)
                {
                    moveNode(cur, curFreq, key);
                    if (curFreq->size == 0)
                    {
                        curFreq->prev->next = curFreq->next;
                        curFreq->next->prev = curFreq->prev;
                        delete curFreq;
                    }
                    return cur->value;
                }
                cur = cur->next;
            }
            return defaultValue;
        }

        virtual void put(const KeyType key, const ValueType value)
        {
            if (capacity == 0)
                return;
            auto iter = nodeMap.find(key);
            if (iter != nodeMap.end())
            {
                FreqNode<KeyType, ValueType> *curFreq = iter->second;
                CacheNode<KeyType, ValueType> *cur = curFreq->head;
                while (cur != nullptr)
                {
                    if (cur->key == key)
                    {
                        cur->value = value;
                        moveNode(cur, curFreq, key);
                        if (curFreq->size == 0)
                        {
                            curFreq->prev->next = curFreq->next;
                            if (curFreq->next != nullptr)
                                curFreq->next->prev = curFreq->prev;
                            delete curFreq;
                        }
                        return;
                    }
                    cur = cur->next;
                }
                return;
            }
            else
            {
                CacheNode<KeyType, ValueType> *newNode = new CacheNode<KeyType, ValueType>(key, value);
                FreqNode<KeyType, ValueType> *targetFreqNode;
                FreqNode<KeyType, ValueType> *firstFreqNode = head->next;
                if (head->next == nullptr || head->next->freq != 1)
                {
                    FreqNode<KeyType, ValueType> *newFreqNode = new FreqNode<KeyType, ValueType>(1, defaultKey, defaultValue);
                    newFreqNode->next = head->next;
                    if (head->next)
                        head->next->prev = newFreqNode;
                    head->next = newFreqNode;
                    newFreqNode->prev = head;
                    targetFreqNode = newFreqNode;
                }
                else
                {
                    targetFreqNode = head->next;
                }

                if (nodeMap.size() == capacity)
                {
                    CacheNode<KeyType, ValueType> *toRemove = firstFreqNode->head->next;
                    firstFreqNode->head->next = toRemove->next;
                    toRemove->next->prev = firstFreqNode->head;
                    nodeMap.erase(toRemove->key);
                    delete toRemove;
                    firstFreqNode->size--;
                    insertNode(newNode, targetFreqNode, key);
                    if (firstFreqNode->size == 0)
                    {
                        firstFreqNode->prev->next = firstFreqNode->next;
                        if (firstFreqNode->next != nullptr)
                            firstFreqNode->next->prev = firstFreqNode->prev;
                        delete firstFreqNode;
                    }
                }
                else
                {
                    insertNode(newNode, targetFreqNode, key);
                }
            }
        }
    };

    template <typename KeyType, typename ValueType>
    class LRUCache : public Cache<KeyType, ValueType>
    {
    private:
        DoubleLinkedList<KeyType, ValueType> *storage;
        int capacity;
        KeyType defaultKey;
        ValueType defaultValue;

    public:
        LRUCache(int capacity, const KeyType defaultKey, const ValueType defaultValue) : capacity(capacity), defaultKey(defaultKey), defaultValue(defaultValue)
        {
            storage = new DoubleLinkedList<KeyType, ValueType>(capacity, defaultKey, defaultValue);
        }

        ~LRUCache() { delete storage; }

        virtual bool contains(const KeyType key)
        {
            if (capacity == 0)
                return false;
            CacheNode<KeyType, ValueType> *cur = storage->head->next;
            while (cur != nullptr)
            {
                if (cur->key == key)
                    return true;
                cur = cur->next;
            }
            return false;
        }

        virtual ValueType get(const KeyType key)
        {
            CacheNode<KeyType, ValueType> *cur = storage->head->next;
            while (cur != nullptr)
            {
                if (cur->key == key)
                {
                    cur->prev->next = cur->next;
                    cur->next->prev = cur->prev;
                    storage->addFirst(cur, true);
                    return cur->value;
                }
                cur = cur->next;
            }
            return defaultValue;
        }

        virtual void put(const KeyType key, const ValueType value)
        {
            CacheNode<KeyType, ValueType> *cur = storage->head->next;
            while (cur != nullptr)
            {
                if (cur->key == key)
                {
                    cur->value = value;
                    cur->prev->next = cur->next;
                    cur->next->prev = cur->prev;
                    storage->addFirst(cur, true);
                    return;
                }
                cur = cur->next;
            }
            storage->addFirst(new CacheNode<KeyType, ValueType>(key, value));
        }
    };
}

#endif