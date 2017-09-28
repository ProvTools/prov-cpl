import os.path
import tornado.escape
from tornado import gen
import tornado.httpserver
import tornado.ioloop
import tornado.options
import tornado.web
import CPL

from tornado.options import define, options

define("port", default=8888, help="run on the given port". type=int)


class Application(tornado.web.Application):
  def __init__(self):
	  handlers = [
      (r"/bundle", BundleHandler, {"cpl_connection" : self.connection}),
      (r"/bundle/info", BundleInfoHandler, {"cpl_connection" : self.connection}),
      (r"/bundle/prefix", BundlePrefixHandler, {"cpl_connection" : self.connection}),
      (r"/bundle/property", BundlePropertyHandler, {"cpl_connection" : self.connection}),
      (r"/bundle/object", BundleObjectHandler, {"cpl_connection" : self.connection}),
      (r"/bundle/relation", BundleRelationHandler, {"cpl_connection" : self.connection}),
      (r"/object", ObjectHandler, {"cpl_connection" : self.connection}),
      (r"object/info", ObjectInfoHandler, {"cpl_connection" : self.connection}),
      (r"/object/property", ObjectPropertyHandler, {"cpl_connection" : self.connection}),
      (r"/object/relation", ObjectRelationHandler, {"cpl_connection" : self.connection}),
      (r"/relation/", RelationHandler, {"cpl_connection" : self.connection}),
      (r"/relation/property", RelationPropertyHandler, {"cpl_connection" : self.connection}),
      (r"/json", JSONHandler, {"cpl_connection" : self.connection})
    ]
    
    super(Application, self).__init__(handlers, **settings)
    
    self.connection = CPL.cpl_connection()


class BundleHandler(tornado.web.RequestHandler):
  def post(self, cpl_connection, name):
    ret = cpl_connection.create_bundle(name)
    self.write(ret)


  def get(self, cpl_connection, name, all):
    if(all):
      bundles = cpl_connection.lookup_all_bundles(name)
      self.write(bundles)
    else:
      bundle = cpl_connection.lookup_bundle(name)
      self.write(bundle)


class BundleInfoHandler(tornado.web.RequestHandler):
  def get(self, cpl_connection, id):
    info = cpl_bundle(id).info()
    self.write(info)


class BundlePrefixHandler(tornado.web.RequestHandler):
  def post(self, id, prefix, iri):
    ret = cpl_bundle(id).add_prefix(prefix, iri)
    self.write(ret)

  def get(self, id, prefix=None):
    ret = cpl_bundle(id).prefixes(prefix)
    self.write(ret)


class BundlePropertyHandler(tornado.web.RequestHandler):
    

class BundleObjectHandler(tornado.web.RequestHandler):


class BundleRelationHandler(tornado.web.ReqestHandler):

    
class ObjectHandler(tornado.web.RequestHandler):
    

class ObjectInfoHandler(tornado.web.RequestHandler):


class ObjectRelationHandler(tornado.web.RequestHandler):


class RelationHandler(tornado.web.RequestHandler):


class RelationPropertyHandler(tornado.web.RequestHandler):


class JSONHandler(tornado.web.RequestHandler):


def main():
  tornado.options.parse_command_line()
  http_server = tornado.httpserver.HTTPServer(Application())
  http_server.listen(options.port)
  tornado.ioloop.IOLoop.current().start() 

if __name__ == "__main__":
  main()
