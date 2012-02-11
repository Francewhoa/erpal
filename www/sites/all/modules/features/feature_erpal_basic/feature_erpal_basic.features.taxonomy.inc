<?php
/**
 * @file
 * feature_erpal_basic.features.taxonomy.inc
 */

/**
 * Implements hook_taxonomy_default_vocabularies().
 */
function feature_erpal_basic_taxonomy_default_vocabularies() {
  return array(
    'company_relation_tags' => array(
      'name' => 'Company relation tags',
      'machine_name' => 'company_relation_tags',
      'description' => 'Describes the position a person has in a company',
      'hierarchy' => '0',
      'module' => 'taxonomy',
      'weight' => '0',
      'rdf_mapping' => array(
        'rdftype' => array(
          0 => 'skos:ConceptScheme',
        ),
        'name' => array(
          'predicates' => array(
            0 => 'dc:title',
          ),
        ),
        'description' => array(
          'predicates' => array(
            0 => 'rdfs:comment',
          ),
        ),
      ),
    ),
    'contact_tags' => array(
      'name' => 'Contact Tags',
      'machine_name' => 'contact_tags',
      'description' => 'Tags for Contacts',
      'hierarchy' => '0',
      'module' => 'taxonomy',
      'weight' => '0',
      'rdf_mapping' => array(
        'rdftype' => array(
          0 => 'skos:ConceptScheme',
        ),
        'name' => array(
          'predicates' => array(
            0 => 'dc:title',
          ),
        ),
        'description' => array(
          'predicates' => array(
            0 => 'rdfs:comment',
          ),
        ),
      ),
    ),
  );
}