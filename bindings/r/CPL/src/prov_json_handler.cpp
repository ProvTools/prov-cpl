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
unsigned long long
import_document_json_r(const char* filename,
					 const char* originator,
					 const char* bundle_name,
					 const unsigned long long anchor_object,
					 const unsigned long long bundle_agent)
{	

	unsigned long long out_id;
	import_document_json(filename, originator, bundle_name, anchor_object, bundle_agent, &out_id);

	return out_id;
}

// [[Rcpp::export]]
void
export_bundle_json_r(const unsigned long long bundle, 
				   const char* path)
{
	
	export_bundle_json(bundle, path)
}

// [[Rcpp::export]]
void
cpl_detach_r(){
	cpl_detach();
}