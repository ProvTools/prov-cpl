import CPL
from CPL import cpl_bundle, cpl_object, cpl_relation
from flask import Flask, request, jsonify

connection = CPL.cpl_connection()

app = Flask(__name__)

def serialize_object_info(obj):
    return {
        'id': obj.object.id,
        'creation_time': obj.creation_time,
        'prefix': obj.prefix,
        'name': obj.name,
        'type': type,
        'bundle': obj.id
    }

def serialize_relation_info(rel):
    return {
        'id': rel.id,
        'ancestor': rel.ancestor.id,
        'descendant': rel.descendant.id,
        'type': rel.type,
        'bundle': rel.id,
        'base': rel.base.id,
        'other': rel.other.id
    }

def serialize_prefix_tuple(pre):
    return {
        'prefix': pre[0],
        'iri': pre[1]
    }

def serialize_property_tuple(prop):
    return {
        'prefix': prop[0],
        'name': prop[1],
        'value': prop[2]
    }

# TODO: wrap returns in make_response with http status codes

@app.route("/provapi/version")
def version_get():
    try:
        return jsonify(version=CPL.CPL_VERSION_STR)
    except Exception as e:
        return jsonify(error=str(e))

@app.route("/provapi/bundle/<id>")
def bundle_get(id):
    try:
        info = cpl_bundle(id).info()
        return jsonify(name=info.name,
                       creation_time=info.creation_time,
                       creation_session=info.creation_session)
    except Exception as e:
        return jsonify(error=str(e))

@app.route("/provapi/bundle", methods=['POST'])
def bundle_post():
    try:
        content = request.get_json()
        bundle = connection.create_bundle(content['name'])
        return jsonify(id=bundle.id)
    except Exception as e:
        return jsonify(error=str(e))

@app.route("/provapi/lookup/bundle", methods=['POST'])
def bundle_lookup():
    try:
        content = request.get_json()
        bundle = connection.lookup_bundle(content['name'])
        return jsonify(id=bundle.id)
    except Exception as e:
        return jsonify(error=str(e))

@app.route("/provapi/lookup/bundles", methods=['POST'])
def bundles_lookup():
    try:
        content = request.get_json()
        bundles = connection.lookup_all_bundles(content['name'])
        return jsonify(ids=[bundle.id for bundle in bundles])
    except Exception as e:
        return jsonify(error=str(e))

@app.route("/provapi/bundle/<id>/prefix")
@app.route("/provapi/bundle/<id>/prefix/<string:prefix>")
def bundle_prefix_get(id, prefix=None):
    try:
        prefixes = cpl_bundle(id).prefixes(prefix)
        return jsonify(prefixes=[serialize_prefix_tuple(pre) for pre in prefixes])
    except Exception as e:
        return jsonify(error=str(e))

@app.route("/provapi/bundle/<id>/prefix", methods=['POST'])
def bundle_prefix_post(id):
    try:
        content = request.get_json()
        cpl_bundle(id).add_prefix(content['prefix'],
                                  content['iri'])
        return jsonify(success=True)
    except Exception as e:
        return jsonify(error=str(e))

@app.route("/provapi/bundle/<id>/property")
@app.route("/provapi/bundle/<id>/property/<string:prefix>:<string:name>")
def bundle_property_get(id, prefix=None, name=None):
    try:
        properties = cpl_bundle(id).properties(prefix, name)
        return jsonify(properties=[serialize_property_tuple(prop) for prop in properties])
    except Exception as e:
        return jsonify(error=str(e))

@app.route("/provapi/bundle/<id>/property", methods=['POST'])
def bundle_property_post(id):
    try:
        content = request.get_json()
        cpl_bundle(id).add_property(content['prefix'],
                                    content['name'],
                                    content['value'])
        return jsonify(success=True)
    except Exception as e:
        return jsonify(error=str(e))

@app.route("/provapi/bundle/<id>/objects")
def bundle_objects_get(id):
    try:
        objects = connection.get_bundle_objects(cpl_bundle(id))
        return jsonify(objects=[serialize_object_info(o) for o in objects])
    except Exception as e:
        return jsonify(error=str(e))

@app.route("/provapi/bundle/<id>/relations")
def bundle_relations_get(id):
    try:
        relations = connection.get_bundle_relations(cpl_bundle(id))
        return jsonify(relations=[serialize_relation_info(r) for r in relations])
    except Exception as e:
        return jsonify(error=str(e))

@app.route("/provapi/object/<id>")
def object_get(id):
    try:
        info = cpl_object(id).info
        return jsonify(serialize_object_info(info))
    except Exception as e:
        return jsonify(error=str(e))

@app.route("/provapi/object", methods=['POST'])
def object_post():
    try:
        content = request.get_json()
        o = connection.create_object(content['prefix'],
                                     content['name'],
                                     content['type'],
                                     cpl_bundle(content['bundle']))
        return jsonify(id=o.id)
    except Exception as e:
        return jsonify(error=str(e))

@app.route("/provapi/lookup/object", methods=['POST'])
def object_lookup():
    try:
        content = request.get_json()
        o = connection.lookup_object(content['prefix'],
                                     content['name'],
                                     content['type'],
                                     cpl_bundle(content['bundle']
                                                if content['bundle'] 
                                                else None))
        return jsonify(id=o.id)
    except Exception as e:
        return jsonify(error=str(e))

@app.route("/provapi/lookup/objects", methods=['POST'])
def objects_lookup():
    try:
        content = request.get_json()
        objects = connection.lookup_all_objects(content['prefix'],
                                                content['name'],
                                                content['type'],
                                                cpl_bundle(content['bundle']
                                                           if content['bundle'] 
                                                           else None))
        return jsonify(ids=[o.id for o in objects])
    except Exception as e:
        return jsonify(error=str(e))    

@app.route("/provapi/object/<id>/property")
@app.route("/provapi/object/<id>/property/<string:prefix>:<string:property>")
def object_property_get(id, prefix=None, property=None):
    try:
        properties = cpl_object(id).properties(prefix, property)
        return jsonify(properties=
            [serialize_property_tuple(prop) for prop in properties])
    except Exception as e:
        return jsonify(error=str(e))


@app.route("/provapi/object/<id>/property")
def object_property_post(id):
    try:
        content = request.get_json()
        cpl_object(id).add_property(content['prefix'],
                                    content['name'],
                                    content['value'])
        return jsonify(success=True)
    except Exception as e:
        return jsonify(error=str(e))

@app.route("/provapi/lookup/object/property", methods=['POST'])
def object_lookup_by_property():
    try:
        content = request.get_json()
        objects = connection.lookup_by_property(content['prefix'],
                                                content['key'],
                                                content['value'])
        return jsonify(ids=[o.id for o in objects])
    except Exception as e:
        return jsonify(error=str(e))

@app.route("/provapi/object/<id>/relation/ancestors")
def object_ancestors_get(id):
    try:
        relations = cpl_object(id).relations()
        return jsonify(relations=[serialize_relation_info(info) for info in relations])
    except Exception as e:
        return jsonify(error=str(e))

@app.route("/provapi/object/<id>/descendants")
def object_descendants_get(id):
    try:
        relations = cpl_object(id).relations(CPL.D_DESCENDANTS)
        return jsonify([serialize_relation_info(info) for info in relations])
    except Exception as e:
        return jsonify(error=str(e))

@app.route("/provapi/object/<id>/relation", methods=['POST'])
def object_relation_post(id):
    try:
        content = request.get_json()
        if content['dest']:
            relation = cpl_object(id).relation_to(cpl_object(content['dest']),
                                                  content['type'],
                                                  cpl_bundle(content['bundle']))
        else:
            relation = cpl_object(id).relation_from(cpl_object(content['src']),
                                                    content['type'],
                                                    cpl_bundle(content['bundle']))
        return jsonify(id=relation.id)
    except Exception as e:
        return jsonify(error=str(e))

@app.route("/provapi/relation/<id>/property")
@app.route("/provapi/relation/<id>/property/<string:prefix>:<string:name>")
def relation_property_get(id, prefix=None, name=None):
    try:
        properties = cpl_relation(id, None, None, None, None, None).properties(prefix, name)
        return jsonify(properties)
    except Exception as e:
        return jsonify(error=str(e))

@app.route("/provapi/relation/<id>/property", methods=['POST'])
def relation_property_post(id):
    try:
        content = request.get_json()
        cpl_relation(id, None, None, None, None, None).add_property(content['prefix'],
                                                                    content['name'],
                                                                    content['value'])
        return jsonify(success=True)
    except Exception as e:
        return jsonify(error=str(e))

@app.route("/provapi/json/validate", methods=['POST'])
def json_validate():
    try:
        content = request.get_json()
        ret = connection.validate_json(content['JSON'])
        if ret == None:
            return jsonify(success=True)
        else:
            return jsonify(error=ret.out_string)
    except Exception as e:
        return jsonify(error=str(e))

@app.route("/provapi/json", methods=['POST'])
def json_post():
    try:
        content = request.get_json()
        anchors = [(cpl_object(a['id']), a['name']) for a in content['anchor_objects']]
        bundle = connection.import_document_json(content['JSON'],
                                        content['bundle_name'],
                                        anchors)
        return jsonify(id=bundle.id)
    except Exception as e:
        return jsonify(error=str(e))

@app.route("/provapi/bundle/<id>/json")
def bundle_json_get(id):
    try:
        json = connection.export_bundle_json([cpl_bundle(id)])
        return jsonify(JSON=json)
    except Exception as e:
        return jsonify(error=str(e))


if __name__ == "__main__":
    app.run()

