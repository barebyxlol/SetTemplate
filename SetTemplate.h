#include <cstddef>
#include <algorithm>

// Template set class, based on AVL-tree. https://en.wikipedia.org/wiki/AVL_tree

template<class T>
class Set {
  private:
    struct node {
        T key;
        size_t height;
        node *left;
        node *right;
        node *parent;
        bool is_end;

        node(T k, node *par) {
            key = k;
            left = nullptr;
            right = nullptr;
            parent = par;
            is_end = false;
            height = 1;
        }

        explicit node(node *par) {
            left = nullptr;
            right = nullptr;
            parent = par;
            is_end = true;
            height = 1;
        }

    };

    // Returns height of a tree vertex.

    size_t GetHeight(node *v) const {
        if (v != nullptr) {
            return v->height;
        }
        return 0;
    }

    // Returns balance factor of a tree vertex.

    int GetBalance(node *v) const {
        if (v != nullptr) {
            return GetHeight(v->left) - GetHeight(v->right);
        }
        return 0;
    }

    // Fixes height field of a vertex, if it is not correct.

    void FixHeight(node *v) {
        if (v != nullptr) {
            v->height = std::max(GetHeight(v->left), GetHeight(v->right)) + 1;
        }
    }

    // Next two methods implement right and left rotation of a vertex to rebalance the tree. Complexity O(1).

    node *RightRotation(node *v) {
        node *q = v->left;
        v->left = q->right;
        if (v->left != nullptr) {
            v->left->parent = v;
        }
        q->right = v;
        q->parent = v->parent;
        v->parent = q;
        FixHeight(v);
        FixHeight(q);
        return q;
    }

    node *LeftRotation(node *v) {
        node *q = v->right;
        v->right = q->left;
        if (v->right != nullptr) {
            v->right->parent = v;
        }
        q->left = v;
        q->parent = v->parent;
        v->parent = q;
        FixHeight(v);
        FixHeight(q);
        return q;
    }

    // Fixes the tree if the current vertex needs to be rebalanced. Complexity O(1).

    node *FixBalance(node *v) {
        if (v == nullptr) {
            return nullptr;
        }
        FixHeight(v);
        if (GetBalance(v) == -2) {
            if (GetBalance(v->right) > 0) {
                v->right = RightRotation(v->right);
            }
            v = LeftRotation(v);
            return v;
        }
        if (GetBalance(v) == 2) {
            if (GetBalance(v->left) < 0) {
                v->left = LeftRotation(v->left);
            }
            v = RightRotation(v);
            return v;
        }
        return v;
    }

    // Inserts a new element into the tree. Complexity O(log n).

    node *Insert(node *v, const T &k, node *par) {
        if (v == nullptr) {
            return new node(k, par);
        }
        if (v->is_end || k < v->key) {
            v->left = Insert(v->left, k, v);
        } else {
            v->right = Insert(v->right, k, v);
        }
        v = FixBalance(v);
        return v;
    }

    // Finds minimal element in the subtree of a current vertex. Complexity O(log n).

    node *FindMin(node *v) const {
        if (v->left == nullptr) {
            return v;
        } else {
            return FindMin(v->left);
        }
    }

    // Finds maximal element in the subtree of a current vertex. Complexity O(log n).

    node *FindEnd(node *v) const {
        if (v->right == nullptr) {
            return v;
        } else {
            return FindEnd(v->right);
        }
    }

    // Erases minimal element in the subtree of the current vertex. Complexity O(log n).

    node *EraseMin(node *v) {
        if (v->left == nullptr) {
            if (v->right != nullptr) {
                v->right->parent = v->parent;
            }
            return v->right;
        }
        v->left = EraseMin(v->left);
        v = FixBalance(v);
        return v;
    }

    // Erases vertex in the tree with the given key value or
    // does nothing if such vertex does not exist. Complexity O(log n).

    node *Erase(node *v, const T &k) {
        if (v == nullptr) {
            return nullptr;
        }
        if (v->is_end || k < v->key) {
            v->left = Erase(v->left, k);
        } else if (v->key < k) {
            v->right = Erase(v->right, k);
        } else {
            node *l = v->left;
            node *r = v->right;
            if (r == nullptr) {
                if (l != nullptr) {
                    l->parent = v->parent;
                }
                delete v;
                return l;
            }
            node *minn = FindMin(r);
            minn->right = EraseMin(r);
            minn->parent = v->parent;
            delete v;
            minn->left = l;
            if (minn->left != nullptr) {
                minn->left->parent = minn;
            }
            if (minn->right != nullptr) {
                minn->right->parent = minn;
            }
            minn = FixBalance(minn);
            return minn;
        }
        v = FixBalance(v);
        return v;
    }

    // Finds a vertex with the given key value or returns nullptr if such vertex does not exist. Complexity O(log n).

    node *Find(node *v, const T &k) const {
        if (v == nullptr) {
            return nullptr;
        }
        if (v->is_end) {
            if (v->left != nullptr && !(v->left->key < k) && !(k < v->left->key)) {
                return v->left;
            } else {
                return nullptr;
            }
        }
        if (k < v->key) {
            return Find(v->left, k);
        } else if (v->key < k) {
            return Find(v->right, k);
        }
        return v;

    }

    // Finds a vertex with the minimal value more or equal to the given key value. Complexity O(log n).

    node *LowerBound(node *v, node *par, const T &k) const {
        if (v == nullptr) {
            return par;
        }
        if (v->is_end) {
            if (v->left != nullptr && !(v->left->key < k)) {
                return v->left;
            }
            return v;
        }
        if (k < v->key) {
            return LowerBound(v->left, v, k);
        } else if (v->key < k) {
            return LowerBound(v->right, v, k);
        }
        return v;
    }

    // Deallocates the memory of the whole tree.

    void DestroySet(node *v) {
        if (v == nullptr) {
            return;
        }
        DestroySet(v->left);
        DestroySet(v->right);
        delete v;
    }

    // Creates a deep copy of a given tree.

    node *CopyNode(node *v, node *par) {
        if (v == nullptr) {
            return nullptr;
        }
        if (v->is_end) {
            node *n = new node(par);
            n->left = CopyNode(v->left, n);
            return n;
        }
        node *n = new node(v->key, par);
        n->left = CopyNode(v->left, n);
        n->right = CopyNode(v->right, n);
        return n;
    }

  public:

    // Iterator class for the set, using pointer to const node to operate.
    // Supports the similar methods as the STL set iterator.

    class iterator {
      public:
        iterator() = default;

        explicit iterator(node *v) : it_(v) {}

        iterator(const iterator &iter) : it_(iter.it_) {}

        iterator &operator=(const iterator &iter) {
            if (this == &iter) {
                return *this;
            }
            it_ = iter.it_;
            return *this;
        }

        bool operator==(const iterator &iter) const {
            return it_ == iter.it_;
        }

        bool operator!=(const iterator &iter) const {
            return it_ != iter.it_;
        }

        // Next four methods implement increments and decrements of an iterator.
        // Incrementing past-the-last element iterator or decrementing first element iterator causes undefined behaviour.
        // The transition to the next element may take up to O(log n) operations, but passage through the entire set
        // takes O(n) operations.


        iterator &operator++() {
            if (it_->is_end) {
                return *this;
            }
            if (it_->right != nullptr) {
                it_ = it_->right;
                while (it_->left != nullptr) {
                    it_ = it_->left;
                }
                return *this;
            }
            T t = it_->key;
            while (!it_->is_end && !(t < it_->key) && it_->parent != nullptr) {
                it_ = it_->parent;
            }
            return *this;
        }

        iterator &operator--() {
            if (it_->is_end) {
                if (it_->left != nullptr) {
                    it_ = it_->left;
                } else {
                    it_ = it_->parent;
                }
                return *this;
            }
            if (it_->left != nullptr) {
                it_ = it_->left;
                while (it_->right != nullptr) {
                    it_ = it_->right;
                }
                return *this;
            }
            T t = it_->key;
            if (it_->parent == nullptr) {
                return *this;
            }
            if (it_->parent->is_end) {
                if (it_->parent->parent == nullptr) {
                    return *this;
                }
                it_ = it_->parent->parent;
            }
            while (!(it_->key < t) && it_->parent != nullptr) {
                it_ = it_->parent;
            }
            return *this;
        }

        iterator &operator++(int) {
            if (it_->is_end) {
                return *this;
            }
            if (it_->right != nullptr) {
                it_ = it_->right;
                while (it_->left != nullptr) {
                    it_ = it_->left;
                }
                return *this;
            }
            T t = it_->key;
            while (!(t < it_->key) && it_->parent != nullptr) {
                it_ = it_->parent;
            }
            return *this;
        }

        iterator &operator--(int) {
            if (it_->is_end) {
                if (it_->left != nullptr) {
                    it_ = it_->left;
                } else {
                    it_ = it_->parent;
                }
                return *this;
            }
            if (it_->left != nullptr) {
                it_ = it_->left;
                while (it_->right != nullptr) {
                    it_ = it_->right;
                }
                return *this;
            }
            T t = it_->key;
            if (it_->parent == nullptr) {
                return *this;
            }
            if (it_->parent->is_end) {
                if (it_->parent->parent == nullptr) {
                    return *this;
                }
                it_ = it_->parent->parent;
            }
            while (!(it_->key < t) && it_->parent != nullptr) {
                it_ = it_->parent;
            }
            return *this;
        }

        T operator*() const {
            return it_->key;
        }

        const T *operator->() const {
            return &(it_->key);
        }

      private:
        const node *it_;
    };

    // Default set constructor.

    Set() {
        root_ = new node(nullptr);
        size_ = 0;
        end_ = root_;
    }

    // Inserts element with the given value to the set.

    void insert(const T &k) {
        if (Find(root_, k) == nullptr) {
            ++size_;
            root_ = Insert(root_, k, nullptr);
        }
    }

    // Constructor from the given sequence of elements specified by the begin and end iterators.

    template<typename Iterator>
    Set(Iterator beginit, Iterator endit) {
        size_ = 0;
        root_ = new node(nullptr);
        end_ = root_;
        std::for_each(beginit, endit, [this](const T &k) { (*this).insert(k); });
    }

    // Initialiser list constructor.

    Set(std::initializer_list<T> lst) {
        size_ = 0;
        root_ = new node(nullptr);
        end_ = root_;
        std::for_each(lst.begin(), lst.end(), [this](const T &k) { (*this).insert(k); });
    }

    // Copy constructor.

    Set(const Set &st) {
        size_ = st.size_;
        root_ = CopyNode(st.root_, nullptr);
        end_ = FindEnd(root_);
    }

    // Copy assignment operator.

    Set &operator=(const Set &st) {
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

    [[nodiscard]] size_t size() const {
        return size_;
    }

    // Returns true if the set is empty.

    [[nodiscard]] bool empty() const {
        return size_ == 0;
    }

    // Returns an iterator to the element with the given key or past-the-end iterator if no such element is found.
    // Complexity O(log n).

    iterator find(T k) const {
        node *v = Find(root_, k);
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

    iterator lower_bound(const T &k) const {
        auto iter = iterator(LowerBound(root_, nullptr, k));
        auto e = iterator(end_);
        while (iter != e && *iter < k) {
            ++iter;
        }
        return iter;
    }

  private:
    node *root_;
    size_t size_;
    node *end_;
};
