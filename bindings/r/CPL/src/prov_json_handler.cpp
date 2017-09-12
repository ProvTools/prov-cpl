#include <cplxx.h>
#include <backends/cpl-odbc.h>
#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
void
cpl_attach_r(){
	cpl_db_backend_t* backend = NULL;
	cpl_create_odbc_backend_dsn("CPL",
						0, &backend);
	cpl_attach(backend);
}

// [[Rcpp::export]]
unsigned long long
import_document_json_r(const std::string& filepath,
					 const std::string& bundle_name)
{	

	unsigned long long out_id;
	import_document_json(filepath, bundle_name, std::vector<std::pair<cpl_id_t, std::string>>(), &out_id);

	return out_id;
}

// [[Rcpp::export]]
void
export_bundle_json_r(const unsigned long long bundle, 
				   const std::string& filepath)
{

	std::vector<cpl_id_t> bundle_vec = {bundle};
	export_bundle_json(bundle_vec, filepath);
}

// [[Rcpp::export]]
void
cpl_detach_r(){
	cpl_detach();
}