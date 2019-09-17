package edu.harvard.pass.cpl;

import swig.direct.CPLDirect.*;

import java.math.BigInteger;

/**
 A relation from a bundle to another relation, indicates that the relation is in the bundle
 */
public class CPLBundleRelation {

    private static final int BUNDLERELATION 		= CPLDirectConstants.BUNDLERELATION;

    /// The null object
    private static BigInteger nullId = BigInteger.ZERO;

    private 	BigInteger id;

    /// The bundle object
    private CPLObject bundle;

    /// The relation
    private CPLRelation relation;

    CPLBundleRelation(BigInteger id){
        this.id = id;
    }

    public static CPLBundleRelation create(CPLObject bundle, CPLRelation relation){
        if (bundle.getType() != CPLDirect.CPL_BUNDLE) {
            throw new CPLException("Cannot create bundle relation from non-bundle", CPLDirect.CPL_E_INVALID_ARGUMENT);
        }
        BigInteger[] id = {nullId};

        int r = CPLDirect.cpl_add_relation(bundle.getId(), relation.getId(), BUNDLERELATION, id);
        CPLException.assertSuccess(r);

        CPLBundleRelation a = new CPLBundleRelation(id[0]);
        a.bundle = bundle;
        a.relation = relation;
        return a;
    }


    /**
     * Get the ID of the relation
     *
     * @return the internal ID of this relation
     */
    public BigInteger getId() {
        return id;
    }
}
