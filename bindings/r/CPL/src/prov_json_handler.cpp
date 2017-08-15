#include <cplxx.h>
#include <rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
void
cpl_attach_r(){
	cpl_db_backend_t* backend = NULL;
	cpl_create_odbc_backend_dsn("CPL",
						CPL_ODBC_POSTGRESQL, &backend);
	cpl_attach(backend);
}

// [[Rcpp::export]]
cpl_id_t
import_document_json_r(const char* filename,
					 const char* originator,
					 const char* bundle_name,
					 const cpl_id_t anchor_object,
					 const cpl_id_t bundle_agent,
					 cpl_id_t* out_id)
{	
	cpl_db_backend_t* backend = NULL;
	cpl_create_odbc_backend_dsn("CPL",
						CPL_ODBC_POSTGRESQL, &backend);
	cpl_return_t ret = cpl_attach(backend);

	cpl_id_t out_id;
	import_document_json(filename, originator, bundle_name, anchor_object, bundle_agent, &out_id);

	return out_id;
}

// [[Rcpp::export]]
void
export_bundle_json_r(const cpl_id_t bundle, 
				   const char* path)
{

	cpl_db_backend_t* backend = NULL;
	cpl_create_odbc_backend_dsn("CPL",
						CPL_ODBC_POSTGRESQL, &backend);
	cpl_attach(backend);

	cpl_id_t out_id;
	import_document_json(filename, originator, bundle_name, anchor_object, bundle_agent, &out_id);


	export_bundle_json_r(bundle, path)
}

// [[Rcpp::export]]
void
cpl_detach_r(){
	cpl_detach();
}