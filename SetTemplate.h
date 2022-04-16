#include <algorithm>
#include <cstddef>

// Template set class, based on AVL-tree. https://en.wikipedia.org/wiki/AVL_tree

template<class T>
class Set {
private:
    struct Node {
        T key;
        size_t height = 1;
        Node* left_son = nullptr;
        Node* right_son = nullptr;
        Node* parent = nullptr;
        bool is_end = false;
        Node(T k, Node* par) {
            key = k;
            parent = par;
        }
        explicit Node(Node* par) {
            parent = par;
            is_end = true;
        }
    };
public:
    // Iterator class for the set, using pointer to const Node to operate.
    // Supports the similar methods as the STL set iterator.
    class iterator {
    public:
        iterator() = default;
        explicit iterator(Node* v) : it_(v) {}
        iterator(const iterator& iter) : it_(iter.it_) {}
        iterator& operator=(const iterator& iter) {
            if (this == &iter) {
                return *this;
            }
            it_ = iter.it_;
            return *this;
        }
        bool operator==(const iterator& iter) const {
            return it_ == iter.it_;
        }
        bool operator!=(const iterator& iter) const {
            return it_ != iter.it_;
        }
        // Next four methods implement increments and decrements of an iterator.
        // Incrementing past-the-last element iterator or decrementing first element iterator causes undefined behaviour.
        // The transition to the next element may take up to O(log n) operations, but passage through the entire set
        // takes O(n) operations.
        iterator& operator++() {
            if (it_->is_end) {
                return *this;
            }
            if (it_->right_son != nullptr) {
                it_ = it_->right_son;
                it_ = GetLeftmost(it_);
                return *this;
            }
            T t = it_->key;
            it_ = GetGreaterParent(it_, t);
            return *this;
        }
        iterator& operator--() {
            if (it_->is_end) {
                it_ = DecreaseEnd(it_);
                return *this;
            }
            if (it_->left_son != nullptr) {
                it_ = it_->left_son;
                it_ = GetRightmost(it_);
                return *this;
            }
            T t = it_->key;
            if (it_->parent == nullptr) {
                return *this;
            }
            it_ = HopEndParent(it_);
            it_ = GetLesserParent(it_, t);
            return *this;
        }
        iterator& operator++(int) {
            if (it_->is_end) {
                return *this;
            }
            if (it_->right_son != nullptr) {
                it_ = it_->right_son;
                it_ = GetLeftmost(it_);
                return *this;
            }
            T t = it_->key;
            it_ = GetGreaterParent(it_, t);
            return *this;
        }
        iterator& operator--(int) {
            if (it_->is_end) {
                it_ = DecreaseEnd(it_);
                return *this;
            }
            if (it_->left_son != nullptr) {
                it_ = it_->left_son;
                it_ = GetRightmost(it_);
                return *this;
            }
            T t = it_->key;
            if (it_->parent == nullptr) {
                return *this;
            }
            it_ = HopEndParent(it_);
            it_ = GetLesserParent(it_, t);
            return *this;
        }
        T operator*() const {
            return it_->key;
        }
        const T* operator->() const {
            return &(it_->key);
        }
    private:
        const Node* DecreaseEnd(const Node* v){
            if (v->left_son != nullptr) {
                return v->left_son;
            } else {
                return v->parent;
            }
        }
        const Node* GetRightmost(const Node* v){
            while (v->right_son != nullptr) {
                v = v->right_son;
            }
            return v;
        }
        const Node* GetLeftmost(const Node* v){
            while (v->left_son != nullptr) {
                v = v->left_son;
            }
            return v;
        }
        const Node* HopEndParent(const Node* v){
            if (v->parent->is_end) {
                if (v->parent->parent == nullptr) {
                    return v;
                }
                v = v->parent->parent;
            }
            return v;
        }
        const Node* GetGreaterParent(const Node* v, const T& t){
            while (!v->is_end && !(t < v->key) && v->parent != nullptr) {
                v = v->parent;
            }
            return v;
        }
        const Node* GetLesserParent(const Node* v, const T& t){
            while (!v->is_end && !(v->key < t) && v->parent != nullptr) {
                v = v->parent;
            }
            return v;
        }
    private:
        const Node* it_;
    };
    // Default set constructor.
    Set() {
        root_ = new Node(nullptr);
        size_ = 0;
        end_ = root_;
    }
    // Inserts element with the given value to the set.
    void insert(const T& k) {
        if (Find(root_, k) == nullptr) {
            ++size_;
            root_ = Insert(root_, k, nullptr);
        }
    }
    // Constructor from the given sequence of elements specified by the begin and end iterators.
    template<typename Iterator>
    Set(Iterator beginit, Iterator endit) {
        size_ = 0;
        root_ = new Node(nullptr);
        end_ = root_;
        std::for_each(beginit, endit, [this](const T& k) { (*this).insert(k); });
    }
    // Initializer list constructor.
    Set(std::initializer_list<T> lst) {
        size_ = 0;
        root_ = new Node(nullptr);
        end_ = root_;
        std::for_each(lst.begin(), lst.end(), [this](const T& k) { (*this).insert(k); });
    }
    // Copy constructor.
    Set(const Set& st) {
        size_ = st.size_;
        root_ = CopyNode(st.root_, nullptr);
        end_ = FindEnd(root_);
    }
    // Copy assignment operator.
    Set& operator=(const Set& st) {
        if (this == &st) {
            return *this;
        }
        DestroySet(root_);
        size_ = st.size_;
        root_ = CopyNode(st.root_, nullptr);
        end_ = FindEnd(root_);
        return *this;
    }
    ~Set() {
        DestroySet(root_);
    }
    // Returns the number of elements in the set.
    size_t size() const {
        return size_;
    }
    // Returns true if the set is empty.
    bool empty() const {
        return size_ == 0;
    }
    // Returns an iterator to the element with the given key or past-the-end iterator if no such element is found.
    // Complexity O(log n).
    iterator find(T k) const {
        Node* v = Find(root_, k);
        if (v == nullptr) {
            return iterator(end_);
        }
        return iterator(v);
    }
    // Erases an element with the given key or does nothing if no such element is found. Complexity O(log n).
    void erase(T k) {
        if (Find(root_, k) != nullptr) {
            --size_;
            root_ = Erase(root_, k);
        }
    }
    // Returns iterator to the first element.
    iterator begin() const {
        return iterator(FindMin(root_));
    }
    // Return past-the-end iterator.
    iterator end() const {
        return iterator(end_);
    }
    // Returns iterator to the first element with the value more or equal to the given key. Complexity O(log n).
    iterator lower_bound(const T& k) const {
        auto iter = iterator(LowerBound(root_, nullptr, k));
        auto e = iterator(end_);
        while (iter != e && *iter < k) {
            ++iter;
        }
        return iter;
    }
private:
    // Returns height of a tree vertex.
    size_t GetHeight(Node* v) const {
        if (v != nullptr) {
            return v->height;
        }
        return 0;
    }
    // Returns balance factor of a tree vertex.
    int32_t GetBalance(Node* v) const {
        if (v != nullptr) {
            return GetHeight(v->left_son) - GetHeight(v->right_son);
        }
        return 0;
    }
    // Fixes height field of a vertex, if it is not correct.
    void FixHeight(Node* v) {
        if (v != nullptr) {
            v->height = std::max(GetHeight(v->left_son), GetHeight(v->right_son)) + 1;
        }
    }
    // Next two methods implement right and left_son rotation of a vertex to rebalance the tree. Complexity O(1).
    Node* RightRotation(Node* v) {
        Node* q = v->left_son;
        v->left_son = q->right_son;
        if (v->left_son != nullptr) {
            v->left_son->parent = v;
        }
        q->right_son = v;
        q->parent = v->parent;
        v->parent = q;
        FixHeight(v);
        FixHeight(q);
        return q;
    }
    Node* LeftRotation(Node* v) {
        Node* q = v->right_son;
        v->right_son = q->left_son;
        if (v->right_son != nullptr) {
            v->right_son->parent = v;
        }
        q->left_son = v;
        q->parent = v->parent;
        v->parent = q;
        FixHeight(v);
        FixHeight(q);
        return q;
    }
    // Fixes the tree if the current vertex needs to be rebalanced. Complexity O(1).
    Node* FixBalance(Node* v) {
        if (v == nullptr) {
            return nullptr;
        }
        FixHeight(v);
        if (GetBalance(v) == -2) {
            if (GetBalance(v->right_son) > 0) {
                v->right_son = RightRotation(v->right_son);
            }
            v = LeftRotation(v);
            return v;
        }
        if (GetBalance(v) == 2) {
            if (GetBalance(v->left_son) < 0) {
                v->left_son = LeftRotation(v->left_son);
            }
            v = RightRotation(v);
            return v;
        }
        return v;
    }
    // Inserts a new element into the tree. Complexity O(log n).
    Node* Insert(Node* v, const T& k, Node* parent) {
        if (v == nullptr) {
            return new Node(k, parent);
        }
        if (v->is_end || k < v->key) {
            v->left_son = Insert(v->left_son, k, v);
        } else {
            v->right_son = Insert(v->right_son, k, v);
        }
        v = FixBalance(v);
        return v;
    }
    // Finds minimal element in the subtree of a current vertex. Complexity O(log n).
    Node* FindMin(Node* v) const {
        if (v->left_son == nullptr) {
            return v;
        } else {
            return FindMin(v->left_son);
        }
    }
    // Finds maximal element in the subtree of a current vertex. Complexity O(log n).
    Node* FindEnd(Node* v) const {
        if (v->right_son == nullptr) {
            return v;
        } else {
            return FindEnd(v->right_son);
        }
    }
    // Erases minimal element in the subtree of the current vertex. Complexity O(log n).
    Node* EraseMin(Node* v) {
        if (v->left_son == nullptr) {
            if (v->right_son != nullptr) {
                v->right_son->parent = v->parent;
            }
            return v->right_son;
        }
        v->left_son = EraseMin(v->left_son);
        v = FixBalance(v);
        return v;
    }
    // Erases vertex in the tree with the given key value or
    // does nothing if such vertex does not exist. Complexity O(log n).
    Node* Erase(Node* v, const T& k) {
        if (v == nullptr) {
            return nullptr;
        }
        if (v->is_end || k < v->key) {
            v->left_son = Erase(v->left_son, k);
        } else if (v->key < k) {
            v->right_son = Erase(v->right_son, k);
        } else {
            Node* l = v->left_son;
            Node* r = v->right_son;
            if (r == nullptr) {
                if (l != nullptr) {
                    l->parent = v->parent;
                }
                delete v;
                return l;
            }
            Node* minnode = FindMin(r);
            minnode->right_son = EraseMin(r);
            minnode->parent = v->parent;
            delete v;
            minnode->left_son = l;
            if (minnode->left_son != nullptr) {
                minnode->left_son->parent = minnode;
            }
            if (minnode->right_son != nullptr) {
                minnode->right_son->parent = minnode;
            }
            minnode = FixBalance(minnode);
            return minnode;
        }
        v = FixBalance(v);
        return v;
    }
    // Finds a vertex with the given key value or returns nullptr if such vertex does not exist. Complexity O(log n).
    Node* Find(Node* v, const T& k) const {
        if (v == nullptr) {
            return nullptr;
        }
        if (v->is_end) {
            if (v->left_son != nullptr && !(v->left_son->key < k) && !(k < v->left_son->key)) {
                return v->left_son;
            } else {
                return nullptr;
            }
        }
        if (k < v->key) {
            return Find(v->left_son, k);
        } else if (v->key < k) {
            return Find(v->right_son, k);
        }
        return v;

    }
    // Finds a vertex with the minimal value more or equal to the given key value. Complexity O(log n).
    Node* LowerBound(Node* v, Node* par, const T& k) const {
        if (v == nullptr) {
            return par;
        }
        if (v->is_end) {
            if (v->left_son != nullptr && !(v->left_son->key < k)) {
                return v->left_son;
            }
            return v;
        }
        if (k < v->key) {
            return LowerBound(v->left_son, v, k);
        } else if (v->key < k) {
            return LowerBound(v->right_son, v, k);
        }
        return v;
    }
    // Deallocates the memory of the whole tree.
    void DestroySet(Node* v) {
        if (v == nullptr) {
            return;
        }
        DestroySet(v->left_son);
        DestroySet(v->right_son);
        delete v;
    }
    // Creates a deep copy of a given tree.
    Node* CopyNode(Node* v, Node* par) {
        if (v == nullptr) {
            return nullptr;
        }
        if (v->is_end) {
            Node* n = new Node(par);
            n->left_son = CopyNode(v->left_son, n);
            return n;
        }
        Node* n = new Node(v->key, par);
        n->left_son = CopyNode(v->left_son, n);
        n->right_son = CopyNode(v->right_son, n);
        return n;
    }
private:
    Node* root_ = nullptr;
    size_t size_ = 0;
    Node* end_ = nullptr;
};
