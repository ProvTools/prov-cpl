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

    CPLBundleRelation(BigInteger id){
        this.id = id;
    }

    public static CPLBundleRelation create(CPLBundle bundle, CPLRelation relation){
        BigInteger[] id = {nullId};

        int r = CPLDirect.cpl_add_relation(bundle.getId(), relation.getId(), BUNDLERELATION, id);
        CPLException.assertSuccess(r);

        CPLBundleRelation a = new CPLBundleRelation(id[0]);
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
