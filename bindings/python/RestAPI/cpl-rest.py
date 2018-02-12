import CPL
from CPL import cpl_bundle, cpl_object, cpl_relation, CPLException
from flask import Flask, request, jsonify
import optparse

connection = CPL.cpl_connection()

app = Flask(__name__)

def serialize_object_info(obj):
    return {
        'id': obj.object.id,
        'creation_time': obj.creation_time,
        'prefix': obj.prefix,
        'name': obj.name,
        'type': CPL.object_type_to_str(obj.type),
        'bundle': obj.id
    }

def serialize_relation_info(rel):
    return {
        'id': rel.id,
        'ancestor': rel.ancestor.id,
        'descendant': rel.descendant.id,
        'type': CPL.relation_type_to_str(rel.type),
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


@app.errorhandler(CPLException)
def handle_cpl_error(e):
    if e.code == CPL.E_INVALID_ARGUMENT or e.code == CPL.E_ALREADY_EXISTS:
        response = jsonify(error=e.msg, success=False)
        response.status_code = 400
        return response
    if e.code == CPL.E_NOT_FOUND:
        response = jsonify(error=e.msg, success=False)
        response.status_code = 404
        return response
    else:
        response = jsonify(error=e.msg, success=False)
        response.status_code = 500
        return response

@app.errorhandler(LookupError)
def handle_lookup_error(e):
    response = jsonify(error="Lookup failed", success=False)
    response.status_code = 404
    return response

@app.errorhandler(Exception)
def handle_exception(e):
    print e
    response = jsonify(error="Unknown internal error", success=False)
    response.status_code = 500
    return response

# TODO: wrap returns in make_response with http status codes
@app.route("/provapi/version")
def version_get():
    return jsonify(version=CPL.CPL_VERSION_STR)

@app.route("/provapi/bundle/<int:id>")
def bundle_get(id):
    info = cpl_bundle(id).info()
    return jsonify(name=info.name,
               creation_time=info.creation_time,
               creation_session=info.creation_session.id)

@app.route("/provapi/bundle/<int:id>", methods=['DELETE'])
def bundle_delete(id):
    connection.delete_bundle(cpl_bundle(id))
    return jsonify(success=True)

@app.route("/provapi/bundle", methods=['POST'])
def bundle_post():
    content = request.get_json()
    bundle = connection.create_bundle(str(content['name']))
    return jsonify(id=bundle.id)

@app.route("/provapi/lookup/bundle", methods=['POST'])
def bundle_lookup():
    content = request.get_json()
    bundle = connection.lookup_bundle(str(content['name']))
    return jsonify(id=bundle.id)

@app.route("/provapi/lookup/bundles", methods=['POST'])
def bundles_lookup():
    content = request.get_json()
    bundles = connection.lookup_all_bundles(str(content['name']))
    return jsonify(ids=[bundle.id for bundle in bundles])

@app.route("/provapi/bundle/<int:id>/prefix")
@app.route("/provapi/bundle/<int:id>/prefix/<string:prefix>")
def bundle_prefix_get(id, prefix=None):
    prefixes = cpl_bundle(id).prefixes(prefix)
    return jsonify(prefixes=[serialize_prefix_tuple(pre) for pre in prefixes])

@app.route("/provapi/bundle/<int:id>/prefix", methods=['POST'])
def bundle_prefix_post(id):
    content = request.get_json()
    cpl_bundle(id).add_prefix(str(content['prefix']),
                              str(content['iri']))
    return jsonify(success=True)

@app.route("/provapi/bundle/<int:id>/property")
@app.route("/provapi/bundle/<int:id>/property/<string:prefix>:<string:name>")
def bundle_property_get(id, prefix=None, name=None):
    properties = cpl_bundle(id).properties(prefix, name)
    return jsonify(properties=[serialize_property_tuple(prop) for prop in properties])

@app.route("/provapi/bundle/<int:id>/property", methods=['POST'])
def bundle_property_post(id):
    content = request.get_json()
    cpl_bundle(id).add_property(str(content['prefix']),
                                str(content['name']),
                                str(content['value']))
    return jsonify(success=True)

@app.route("/provapi/bundle/<int:id>/objects")
def bundle_objects_get(id):
    objects = connection.get_bundle_objects(cpl_bundle(id))
    return jsonify(objects=[serialize_object_info(o) for o in objects])

@app.route("/provapi/bundle/<int:id>/relations")
def bundle_relations_get(id):
    relations = connection.get_bundle_relations(cpl_bundle(id))
    return jsonify(relations=[serialize_relation_info(r) for r in relations])

@app.route("/provapi/object/<int:id>")
def object_get(id):
    info = cpl_object(id).info
    return jsonify(serialize_object_info(info))

@app.route("/provapi/object", methods=['POST'])
def object_post():
    content = request.get_json()
    type_int = CPL.object_str_to_type(str(content['type']))
    o = connection.create_object(str(content['prefix']),
                                 str(content['name']),
                                 type_int,
                                 cpl_bundle(int(content['bundle'])))
    return jsonify(id=o.id)

@app.route("/provapi/lookup/object", methods=['POST'])
def object_lookup():
    content = request.get_json()
    type_int = CPL.object_str_to_type(str(content['type']))
    o = connection.lookup_object(str(content['prefix']),
                                 str(content['name']),
                                 type_int,
                                 cpl_bundle(str(content['bundle'])
                                            if content['bundle'] 
                                            else None))
    return jsonify(id=o.id)

@app.route("/provapi/lookup/objects", methods=['POST'])
def objects_lookup():
    content = request.get_json()
    type_int = CPL.object_str_to_type(str(content['type']))
    objects = connection.lookup_all_objects(str(content['prefix']),
                                            str(content['name']),
                                            type_int,
                                            cpl_bundle(str(content['bundle'])
                                                       if content['bundle'] 
                                                       else None))
    return jsonify(ids=[o.id for o in objects])    

@app.route("/provapi/object/<int:id>/property")
@app.route("/provapi/object/<int:id>/property/<string:prefix>:<string:property>")
def object_property_get(id, prefix=None, property=None):
    properties = cpl_object(id).properties(prefix, property)
    return jsonify(properties=
        [serialize_property_tuple(prop) for prop in properties])


@app.route("/provapi/object/<int:id>/property")
def object_property_post(id):
    content = request.get_json()
    cpl_object(id).add_property(str(content['prefix']),
                                str(content['name']),
                                str(content['value']))
    return jsonify(success=True)

@app.route("/provapi/lookup/object/property", methods=['POST'])
def object_lookup_by_property():
    content = request.get_json()
    objects = connection.lookup_by_property(str(content['prefix']),
                                            str(content['key']),
                                            str(content['value']))
    return jsonify(ids=[o.id for o in objects])

@app.route("/provapi/object/<int:id>/relation/ancestors")
def object_ancestors_get(id):
    relations = cpl_object(id).relations()
    return jsonify(relations=[serialize_relation_info(info) for info in relations])

@app.route("/provapi/object/<int:id>/descendants")
def object_descendants_get(id):
    relations = cpl_object(id).relations(CPL.D_DESCENDANTS)
    return jsonify([serialize_relation_info(info) for info in relations])

@app.route("/provapi/object/<int:id>/relation", methods=['POST'])
def object_relation_post(id):
    content = request.get_json()
    type_int = CPL.relation_str_to_type(str(content['type']))
    if content['dest']:
        relation = cpl_object(id).relation_to(cpl_object(int(content['dest'])),
                                              type_int,
                                              cpl_bundle(int(content['bundle'])))
    else:
        relation = cpl_object(id).relation_from(cpl_object(int(content['src'])),
                                                type_int,
                                                cpl_bundle(int(content['bundle'])))
    return jsonify(id=relation.id)

@app.route("/provapi/relation/<int:id>/property")
@app.route("/provapi/relation/<int:id>/property/<string:prefix>:<string:name>")
def relation_property_get(id, prefix=None, name=None):
    properties = cpl_relation(id, None, None, None, None, None).properties(prefix, name)
    return jsonify(properties)

@app.route("/provapi/relation/<int:id>/property", methods=['POST'])
def relation_property_post(id):
    content = request.get_json()
    cpl_relation(id, None, None, None, None, None).add_property(str(content['prefix']),
                                                                str(content['name']),
                                                                str(content['value']))
    return jsonify(success=True)

@app.route("/provapi/json/validate", methods=['POST'])
def json_validate():
    content = request.get_json()
    ret = connection.validate_json(content['JSON'])
    if ret == None:
        return jsonify(success=True)
    else:
        return jsonify(error=ret.out_string)

@app.route("/provapi/json", methods=['POST'])
def json_post():
    content = request.get_json()
    if content.get('anchor_objects'):
        anchors = [(cpl_object(int(a['id'])), str(a['name'])) for a in content['anchor_objects']]
    else:
        anchors = None
    print content['JSON']
    bundle = connection.import_document_json(str(content['JSON']),
                                    str(content['bundle_name']),
                                    anchors)
    return jsonify(id=bundle.id)

@app.route("/provapi/bundle/<int:id>/json")
def bundle_json_get(id):
    json = connection.export_bundle_json([cpl_bundle(id)])
    return jsonify(JSON=json)

def flaskrun(app, default_host="127.0.0.1", 
                  default_port="5000"):
    """
    Takes a flask.Flask instance and runs it. Parses 
    command-line flags to configure the app.
    """

    # Set up the command-line options
    parser = optparse.OptionParser()
    parser.add_option("-H", "--host",
                      help="Hostname of the Flask app " + \
                           "[default %s]" % default_host,
                      default=default_host)
    parser.add_option("-P", "--port",
                      help="Port for the Flask app " + \
                           "[default %s]" % default_port,
                      default=default_port)

    options, _ = parser.parse_args()

    app.run(
        host=options.host,
        port=int(options.port)
    )

if __name__ == "__main__":
    flaskrun(app)

