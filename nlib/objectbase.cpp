// Unit for controls only used in the designer. These controls will not be visible while the programs run, instead provide data management, dialog boxes etc.
#include "stdafx_zoli.h"

#include "objectbase.h"
#include "imagelist.h"
#include "dialog.h"

#ifdef DESIGNING
#include "designer.h"
#include "designregistry.h"
#include "serializer.h"
#include "property_objectbase.h"
#include "designerform.h"
#endif


//---------------------------------------------


namespace NLIBNS
{


#ifdef DESIGNING
    ValuePair<AccessLevels> AccessLevelStrings[] = {
        VALUEPAIR(alPrivate),
        VALUEPAIR(alProtected),
        VALUEPAIR(alPublic),
    };

    //void ParentSetterDesignPropertySerializerFunc(Indentation &indent, const std::wstring& prefix, bool pointerprefix, std::wiostream &ws, Object *propholder, DesignProperty *prop, int resnum)
    //{
    //    ws << indent << prefix << (pointerprefix ? L"->" : L".") << L"SetParent(this);" << std::endl;
    //}

    void Object::EnumerateProperties(DesignSerializer *serializer)
    {
        serializer->Add(std::wstring(), new NameDesignProperty<Object>() )->SetPriority(1000)->DisableSharedEdit()->MakeDefault();
        serializer->Add(L"SetTag", new TagDesignProperty<Object>(L"Tag", L"User data", &Object::Tag, &Object::SetTag) )->SetDefault(0);
        serializer->Add(L"SetAccessLevel", new AccessLevelDesignProperty<Object>(L"AccessLevel", std::wstring(), &Object::AccessLevel, &Object::SetAccessLevel))->DontExport();
    }

    DesignFormBase* Object::DesignParent()
    {
        DesignFormBase* form = dynamic_cast<DesignFormBase*>(ParentForm());
        if (!form)
            return dynamic_cast<DesignFormBase*>(this);
        return form;
    }

    std::wstring Object::ClassName(bool namespacedname)
    {
        return DisplayNameByTypeInfo(typeid(*PropertyOwner()), namespacedname);
    }

    const type_info& Object::TypeInfo()
    {
        return typeid(*PropertyOwner());
    }

    AccessLevels Object::AccessLevel()
    {
        return access;
    }

    void Object::SetAccessLevel(AccessLevels newacc)
    {
        access = newacc;
    }

    bool Object::Designing() const
    {
        return designing;
    }

    void Object::SetDesigning()
    {
        designing = true;
    }

    Object::Object() : access(GetSettings().control_access), designing(false),
#else
    Object::Object() :
#endif
        tag(0)
    {
    }

    /**
     * Objects are not copy constructible, so the copy constructor is protected in most derived classes as well.
     */
#ifdef DESIGNING
    Object::Object(const Object &orig) : name(orig.name),
#else
    Object::Object(const Object &orig) :
#endif
        tag(orig.tag)
    {
    }

    /**
     * Instead of `delete`, call Destroy() if you want to delete the object. The destructor should be protected in
     * derived classes as well to avoid calling `delete`, which gets around the proper finalization.
     */
    Object::~Object()
    {
    }

    /**
     * Call this function when the object must be destroyed to do any cleanup, instead of making the destructor
     * public in derived classes. The destructor should be protected in derived classes, and the cleanup code
     * should be put in the virtual Destroy() method instead. Always call the parent's Destroy() method last,
     * after any members were accessed. The base class removes itself from the notify list of other objects, and
     * calls `delete this`.
     */
    void Object::Destroy()
    {
        if (objectstate.contains(osDestroying))
            return;

        if (objectstate.contains(osNotifying))
            throw L"Cannot destroy object during change notification";

        objectstate << osDestroying;

        deleteiterator = notifylist.begin();
        while(deleteiterator != notifylist.end())
        {
            Object *obj = deleteiterator->first;
            deleteiterator = notifylist.erase(deleteiterator);
            obj->DeleteNotify(this);
        }

#ifdef DESIGNING
        if (SubOwner() != nullptr && SubOwner()->Designing() && SubOwner()->DesignParent() != nullptr)
            SubOwner()->DesignParent()->UnregisterSubObject(SubOwner(), this);
#endif

        delete this;
    }

    /**
     * Override in derived classes that need to do cleanup when an Object in their notify list is being deleted.
     * \param object The object which is being deleted, which called the method.
     */
    void Object::DeleteNotify(Object *object)
    {
        auto it = std::find_if(notifylist.begin(), notifylist.end(), [&object](std::pair<Object*, NotifyRelationSet> &val) { return val.first == object; } );
        if (it == notifylist.end())
            return;

        EraseFromNotify(it, nrNoReason);
    }

    /**
     * The definition of change depends on the object. Some objects use Change() when their items are updated,
     * others when their stored data (e.g. an image) changes. The base implementation does nothing.
     * \param object The object being changed.
     * \param changetype The identifier of the change that occured. Usually 0 when only a single kind of change is possible.
     */
    void Object::ChangeNotify(Object *object, int changetype)
    {
    }

    void Object::EraseFromNotify(std::list<std::pair<Object*, NotifyRelationSet>>::iterator it, NotifyRelations relation)
    {
        it->second -= relation;

        if (it->second.empty() || relation == nrNoReason)
        {
            bool samedel = objectstate.contains(osDestroying) && it == deleteiterator;
            bool samechn = objectstate.contains(osNotifying) && it == changeiterator;

            auto nextit = notifylist.erase(it);

            if (objectstate.contains(osDestroying) && samedel)
                deleteiterator = nextit;
            if (objectstate.contains(osNotifying) && samechn)
                changeiterator = nextit;
        }
    }

    /**
     * Makes to objects related by adding each other in their notify lists when some change in one of
     * them requires some measures to be taken by the other object. For example when an image must be
     * updated on a button.
     * \param object The object to add to the notify list.
     * \param relation The relation between the passed object and this one.
     * \return \a true if the controls were not in each other's notify list. The return value is \a false even if the relation didn't match with existing relations between the objects.
     */
    bool Object::AddToNotifyList(Object *object, NotifyRelations relation)
    {
        if (!object)
            return false;
        if (relation == nrNoReason)
            throw L"Cannot add to notify list with no reason.";

        auto it = std::find_if(notifylist.begin(), notifylist.end(), [object](std::pair<Object*, NotifyRelationSet> &val) { return val.first == object; } );
        if (it == notifylist.end())
        {
            AddToNotify(object, relation);
            object->AddToNotifyList(this, relation);
            return true;
        }

        it->second << relation;
        return false;
    }

    void Object::AddToNotify(Object *object, NotifyRelations relation)
    {
        notifylist.push_back(std::make_pair(object, relation));
        if (objectstate.contains(osDestroying) && deleteiterator == notifylist.end())
            --deleteiterator;

        if (objectstate.contains(osNotifying) && changeiterator == notifylist.end())
            --changeiterator;
    }

    /**
     * If objects are in each other's notify list with multiple relations, only that relation will be removed.
     * \param object The object in relation to this one to remove.
     * \param relation The relation which will be invalidated. Pass nrNoReason to remove all relations between the objects.
     */
    void Object::RemoveFromNotifyList(Object *object, NotifyRelations relation)
    {
        if (!object)
            return;

        for (auto it = notifylist.begin(); it != notifylist.end(); ++it)
        {
            if (it->first == object)
            {
                if (relation == nrNoReason || it->second == relation)
                {
                    EraseFromNotify(it, relation);
                    object->RemoveFromNotifyList(this, relation);
                }
                else
                    it->second -= relation;
                break;
            }
        }
    }

    /**
     * \param object The object in relation with this one to look for.
     * \param relation The relation to look for between the objects. Pass nrNoReason if the relation doesn't matter.
     */
    bool Object::InNotifyList(Object *object, NotifyRelations relation)
    {
        for (auto it = notifylist.begin(); it != notifylist.end(); ++it)
        {
            if (it->first == object)
            {
                if (relation == nrNoReason || it->second.contains(relation))
                    return true;
                break;
            }
        }
        return false;
    }

    /**
     * Derived controls can override this function if they depend on changes of the user defined data.
     * The function is called after the tag change. The base implementation does nothing.
     * \param oldtag The previous value of the user defined data.
     */
    void Object::TagChanged(tagtype oldtag)
    {
    }

    const ObjectStateSet& Object::ObjectState()
    {
        return objectstate;
    }

    /**
     * Objects of derived classes call Changed() when they want to notify another object of some kind of change.
     * For example ControlImage objects call it when their owner control must be visually updated to reflect the
     * current state of the image.
     * \sa ObjectStates
     */
    void Object::Changed(int changetype)
    {
        if (objectstate.contains(osNotifying))
            throw L"Cannot access the notifylist during change";

        objectstate << osNotifying;

        try
        {
            Object *tmpobj;
            changeiterator = notifylist.begin();
            while (changeiterator != notifylist.end())
            {
                tmpobj = changeiterator->first;
                tmpobj->ChangeNotify(this, changetype);
                if (changeiterator != notifylist.end() && tmpobj == changeiterator->first)
                    ++changeiterator;
            }
        }
        catch(...)
        {
            objectstate -= osNotifying;
            throw;
        }

        objectstate -= osNotifying;
    }

#ifdef DESIGNING
    void Object::NameChanged(const std::wstring &oldname)
    {
        if (objectstate.contains(osNotifying))
            throw L"Cannot access the notifylist during change";

        objectstate << osNotifying;

        if (SubOwner() != nullptr && SubOwner()->Designing() && SubOwner()->DesignParent() != nullptr)
        {
            if (oldname.empty())
                SubOwner()->DesignParent()->RegisterSubObject(SubOwner(), this, name);
            else
                SubOwner()->DesignParent()->SubObjectRenamed(SubOwner(), this, oldname, name);
        }

        try
        {
            Object *tmpobj;
            changeiterator = notifylist.begin();
            while (changeiterator != notifylist.end())
            {
                tmpobj = changeiterator->first;
                tmpobj->NameChangeNotify(this, oldname);
                if (changeiterator != notifylist.end() && tmpobj == changeiterator->first)
                    ++changeiterator;
            }
        }
        catch(...)
        {
            objectstate -= osNotifying;
            throw;
        }

        objectstate -= osNotifying;
    }
#endif

    /**
     * For Objects without a parent form, this function can return null.
     * If the object was derived from Control, this is the form which displays it.
     */
    Form* Object::ParentForm() const
    {
        return nullptr;
    }

    tagtype Object::Tag() const
    {
        return tag;
    }

    void Object::SetTag(tagtype newtag)
    {
        tagtype old = tag;
        tag = newtag;
        if (old != tag)
            TagChanged(old);
    }

#ifdef DESIGNING
    const std::wstring& Object::Name() const
    {
        return name;
    }

    Object* Object::PropertyOwner()
    {
        return this;
    }

    Object* Object::MainControl() 
    {
        return nullptr;
    }

    //Object* Object::NameOwner(const std::wstring& aname)
    //{
    //    if (name == aname)
    //        return this;
    //    return nullptr;
    //    //std::vector<std::wstring> namelist;
    //    //Names(namelist);
    //    //for (auto it = namelist.begin(); it != namelist.end(); ++it)
    //    //    if ((*it) == aname)
    //    //        return true;
    //    //return false;
    //}

    void Object::SetName(const std::wstring& newname)
    {
        std::wstring oldname = name;

        name = newname;
        NameChanged(oldname);
    }

    Object* Object::SubOwner()
    {
        return nullptr;
    }

    bool Object::SubShown()
    {
        return true;
    }

    DesignSerializer* Object::Serializer()
    {
        return SerializerByTypeInfo(typeid(*PropertyOwner()));
    }

#endif


    //---------------------------------------------


#ifdef DESIGNING


    NonVisualSubItem::NonVisualSubItem(const std::wstring& propname, Object *item) : propname(propname), item(item)
    {
    }
    
    NonVisualSubItem::NonVisualSubItem(NonVisualSubItem &&orig) : propname(std::move(orig.propname)), subitems(std::move(orig.subitems)), item(orig.item)
    {
        orig.propname.clear();
        orig.item = 0;
    }


    //---------------------------------------------


    void NonVisualControl::AddParent(DesignFormBase *newparent)
    {
        if (newparent == nullptr || std::find(parents.begin(), parents.end(), newparent) != parents.end())
            return;
        parents.push_back(newparent);

        if (parent == nullptr)
            SetParent(newparent);

        Changed(CHANGE_PARENT);
    }

    void NonVisualControl::RemoveParent(DesignFormBase *oldparent)
    {
        if (oldparent == nullptr)
            return;

        auto it = std::find(parents.begin(), parents.end(), oldparent);
        if (it == parents.end())
            return;
        parents.erase(it);

        //if (parent == oldparent)
        //    SetParent(nullptr);

        Changed(CHANGE_PARENT);
    }

    int NonVisualControl::ParentCount()
    {
        return parents.size();
    }

    DesignFormBase* NonVisualControl::Parents(int ix)
    {
        return parents[ix];
    }

    bool NonVisualControl::IsParent(DesignFormBase *form)
    {
        for (DesignFormBase *f : parents)
            if (f == form)
                return true;
        return false;
    }

    void NonVisualControl::EnumerateProperties(DesignSerializer *serializer)
    {
        base::EnumerateProperties(serializer);
        //serializer->Add(std::wstring(), new ParentFormSetterDesignProperty<NonVisualControl>());
    }
#endif

    NonVisualControl::NonVisualControl() : parent(NULL)
    {
    }

    /**
     * Call Destroy() if you want to delete the object, instead of calling delete. The destructor must be protected in derived classes as well.
     */
    NonVisualControl::~NonVisualControl()
    {
    }

    void NonVisualControl::Destroy()
    {
#ifdef DESIGNING
        while (ParentCount() > 0)
            RemoveParent(Parents(ParentCount() - 1));
#endif

        base::Destroy();
    }

    /**
     * Either a Form or a Container can be specified for the control, not both. For objects without a parent Form, this function returns NULL.
     * \sa ParentContainer
     */
    Form* NonVisualControl::ParentForm() const
    {
        return dynamic_cast<Form*>(parent);
    }

    /**
     * Either a Form or a Container can be specified for the control, not both. For objects without a parent Container, this function returns NULL.
     * \sa ParentForm
     */
    Container* NonVisualControl::ParentContainer()
    {
        return dynamic_cast<Container*>(parent);
    }

    /**
     * If the object had a parent before calling this function, it will be replaced.
     * \param newparent The new parent to be set as the parent for the object. It must either be a Form or a Container, otherwise NULL will be set as the new parent.
     */
    void NonVisualControl::SetParent(Object *newparent)
    {
        if (parent == newparent)
            return;

        if (dynamic_cast<Form*>(parent))
        {
            Form *f = dynamic_cast<Form*>(parent);
            f->RemoveNVChild(this);
        }
        else if (parent)
        {
            Container *c = dynamic_cast<Container*>(parent);
            c->RemoveNVChild(this);
        }
        parent = newparent;
        if (dynamic_cast<Form*>(newparent))
        {
            Form *f = dynamic_cast<Form*>(newparent);
            f->AddNVChild(this);
        }
        else if (newparent)
        {
            Container *c = dynamic_cast<Container*>(newparent);
            c->AddNVChild(this);
        }

#ifdef DESIGNING
        if (dynamic_cast<DesignFormBase*>(newparent) != nullptr)
            AddParent((DesignFormBase*)newparent);
#endif
    }


    //---------------------------------------------


    Container::Container()
    {
    }

    Container::~Container()
    {
    }

    void Container::Destroy()
    {
        while (nvs.size())
        {
            auto c = nvs.front();
            c->Destroy();
            if (!nvs.empty() && c == nvs.front())
                nvs.erase(nvs.begin());
        }
        base::Destroy();
    }

    void Container::DeleteNotify(Object *object)
    {
        base::DeleteNotify(object);

        if (dynamic_cast<NonVisualControl*>(object) != NULL)
        {
            for (auto it = nvs.begin(); it != nvs.end(); ++it)
            {
                if (*it == object)
                {
                    nvs.erase(it);
                    break;
                }
            }
        }
    }

    /**
     * The container will delete its NonVisualControl children when it is destroyed.
     * \param nv The NonVisualControl to be added as child.
     */
    void Container::AddNVChild(NonVisualControl *nv)
    {
        if (AddToNotifyList(nv, nrOwnership))
            nvs.push_back(nv);
    }

    /**
     * The container will delete its NonVisualControl children when it is destroyed.
     * \param nv The NonVisualControl to be removed from the children.
     */
    void Container::RemoveNVChild(NonVisualControl *nv)
    {
        RemoveFromNotifyList(nv, nrOwnership);
        for (auto it = nvs.begin(); it != nvs.end(); ++it)
        {
            if (*it == nv)
            {
                nvs.erase(it);
                break;
            }
        }
    }


    //---------------------------------------------


}
/* End of NLIBNS */

