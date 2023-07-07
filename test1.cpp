#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <queue>
#include <set>
using namespace std;

class Node
{
public:
    string s;
    bool isLocked;
    int id;
    Node *parent;
    int anc_locked;
    int dec_locked;
    vector<Node *> children;
    set<Node *> locked_desc_list;
    int cnt_desc_nodes_in_thread;
    bool node_in_use_in_thread;
    Node()
    {
        isLocked = false;
        id = -1;
        parent = NULL;
        anc_locked = 0;
        dec_locked = 0;
        cnt_desc_nodes_in_thread = 0;
        node_in_use_in_thread = false;
    }
};

void printTree(Node *root)
{
    cout << root->s << "\n"
         << "is root locked: " << root->isLocked << "\n"
         << "parent: " << (root->parent != NULL ? root->parent->s : "root node") << "\n"
         << "locked ancestor count: " << root->anc_locked << "\n"
         << "locked descendent count: " << root->dec_locked << "\n"
         << "id: " << root->id << "\n";
    cout << "locked children list start" << endl;
    for (auto itr = root->locked_desc_list.begin(); itr != root->locked_desc_list.end(); itr++)
    {
        cout << (*itr)->s << endl;
    }
    cout << "=====================" << endl;
    for (auto child : root->children)
    {
        printTree(child);
    }
}

int check_anc_locked_count(Node *node)
{
    int count = 0;
    while (node != NULL)
    {
        if (node->isLocked)
        {
            count++;
        }
        node = node->parent;
    }
    return count;
}

//===============================================

void inform_ancestors_to_add(Node *node, int val, Node *desc_locked)
{

    while (node != NULL)
    {
        node->dec_locked += 1;
        (node->locked_desc_list).insert(desc_locked);
        node = node->parent;
    }
}

bool flag = true; // 1 means no thread is in the check function i.e we are good to go

bool check_for_thread_do_operation(Node *node)
{
    if (flag == 0)
        return false;
    flag = 0;
    if (node->cnt_desc_nodes_in_thread != 0)
    {
        flag = true;
        return false;
    }
    // check if node_in_use_in_thread =false for all ancestors from parent to root
    Node *ancestor = node->parent;
    while (ancestor != NULL)
    {
        if (ancestor->node_in_use_in_thread)
        {
            flag = true;
            return false;
        }
        ancestor = ancestor->parent;
    }

    ancestor = node->parent;
    while (ancestor != NULL)
    {
        ancestor->cnt_desc_nodes_in_thread += 1;
        ancestor = ancestor->parent;
    }

    node->node_in_use_in_thread = true;
    flag = true;
    return true;
}

bool check_for_thread_undo_operation(Node *node)
{
    if (flag == false)
    {
        return false;
    }
    flag = false;
    Node *ancestor = node->parent;
    while (ancestor != NULL)
    {
        ancestor->cnt_desc_nodes_in_thread -= 1;
        ancestor = ancestor->parent;
    }
    node->node_in_use_in_thread = false;
    flag = true;
    return true;
}

bool lock(Node *node, int id)
{

    while (!check_for_thread_do_operation(node))
        ;

    if (node->isLocked)
    {
        return false;
    }
    if (node->dec_locked > 0)
    {
        return false;
    }
    if (check_anc_locked_count(node->parent) > 0)
    {
        return false;
    }

    node->isLocked = true;
    node->id = id;
    inform_ancestors_to_add(node->parent, 1, node);

    while (!check_for_thread_undo_operation(node))
        ;

    return true;
}

//=========================================

void inform_ancestors_to_remove(Node *node, int val, Node *desc_unlocked)
{
    while (node != NULL)
    {
        node->dec_locked += val;
        node->locked_desc_list.erase(desc_unlocked);
        node = node->parent;
    }
}

bool unlock(Node *node, int id)
{
    while (!check_for_thread_do_operation(node))
        ;
    if (node->isLocked == false || node->id != id)
    {
        return false;
    }

    node->isLocked = false;
    node->id = -1;
    inform_ancestors_to_remove(node->parent, -1, node);
    while (!check_for_thread_undo_operation(node))
        ;
    return true;
}

bool upgrade(Node *node, int id)
{
    while (!check_for_thread_do_operation(node))
        ;
    if (node->isLocked)
    {
        return false;
    }

    if (node->dec_locked == 0)
    {
        return false;
    }
    set<int> ids;
    for (auto it = node->locked_desc_list.begin(); it != node->locked_desc_list.end(); ++it)
    {
        if ((*it)->isLocked)
        {
            ids.insert((*it)->id);
        }
    }

    if (ids.size() > 1)
    {
        return false;
    }

    if (ids.size() == 0)
    {
        return false;
    }

    bool res = true;
    set<Node *> temp;

    for (auto it = node->locked_desc_list.begin(); it != node->locked_desc_list.end(); ++it)
    {
        temp.insert(*it);
    }

    if (temp.size() == 0)
    {
        return false;
    }
    for (auto it = temp.begin(); it != temp.end(); ++it)
    {
        unlock(*it, id);
    }

    while (!check_for_thread_undo_operation(node))
        ;
    return lock(node, id);
}

int main()
{

    int n;
    cin >> n;
    int k;
    cin >> k;
    int q;
    cin >> q;

    map<string, Node *> mp;
    vector<string> v(n);

    for (int i = 0; i < n; i++)
    {
        cin >> v[i];
    }

    queue<Node *> qu;
    Node *root = new Node();
    root->s = v[0];
    mp[v[0]] = root;
    int index = 1;
    qu.push(root);

    while (!qu.empty() && index < n)
    {
        int size = qu.size();
        while (size--)
        {
            Node *rem = qu.front();
            qu.pop();
            for (int i = 1; i <= k && index < n; i++)
            {
                Node *newNode = new Node();
                newNode->parent = rem;
                mp[v[index]] = newNode;
                newNode->s = v[index];
                (rem->children).push_back(newNode);
                qu.push(newNode);
                index++;
            }
        }
    }

    for (int i = 0; i < q; i++)
    {
        int val, id;
        string s;
        cin >> val >> s >> id;

        bool ans = false;
        if (val == 1)
        {
            ans = lock(mp[s], id);
        }
        else if (val == 2)
        {
            ans = unlock(mp[s], id);
        }
        else if (val == 3)
        {
            ans = upgrade(mp[s], id);
        }

        cout << (ans ? "true" : "false") << endl;
    }
}