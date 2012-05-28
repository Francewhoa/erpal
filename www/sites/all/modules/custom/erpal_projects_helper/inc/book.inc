<?php

/**
* Implements hook_book_child_nodes provided by erpal_book_helper module
*/
function erpal_projects_helper_book_child_nodes($parent_node, $book_node, $options) {
  
  if (!$options['export_tasks'])
    return array();
  
  //we return all tasks that have the given parent_node as parent
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'node')
  ->entityCondition('bundle', 'erpal_task')
  ->propertyCondition('status', 1)
  ->fieldCondition('field_parent', 'nid', $parent_node->nid, '=')
  ->fieldOrderBy('field_weight', 'value', 'ASC');

  if (!$options['ignore_exclude']['pdf']) {
    //exclude some node from export
    $exclude = array('pdf');
    
    //normally we would do it lik
    //$query->fieldCondition('field_task_exclude_from', 'value', 'pdf', '!=');
    //but that doenst work if field has no value, not even NULL at all. So we get all excluded nids and
    //exclude them in the query.
    $field['field'] = 'field_task_exclude_from';
    $field['column'] = 'value';
    $excluded = _erpal_book_helper_get_excluded_nodes_by_parent($parent_node, $exclude, $field);

    if (count($excluded) > 0)
      $query->propertyCondition('nid', $excluded, 'NOT IN');
  } 
  
  $result = $query->execute();

  $child_task_nodes = array();
  if (isset($result['node'])) {
    $child_nids = array_keys($result['node']);
    $child_task_nodes = entity_load('node', $child_nids);
  }

  return $child_task_nodes;
}

/**
* Return all nids of thats that are excluded for a given aggregation of tasks
*/
function _erpal_projects_helper_get_excluded_task_nids($project_node, $exclude) {
  
  if (!$exclude)
    return array();
    
  if ($exclude && !is_array($exclude))
    $exclude = array($exclude);
  
  //first get all book pages
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'node')
  ->entityCondition('bundle', 'erpal_task')
  ->propertyCondition('status', 1)
  ->fieldCondition('field_book_ref', 'nid', $book_node->nid, '=')
  ->fieldCondition('field_project_ref', 'nid', $project_node->nid, '=')
  ->fieldCondition('field_task_exclude_from', 'value', $exclude, 'IN');
  
  $result = $query->execute();
  $excluded_nids = array();
  if (isset($result['node'])) {
    $excluded_nids = array_keys($result['node']);
  }
  
  return $excluded_nids;
}

/**
* alter pdf create form provided by erpal_book_helper module
*/
function _erpal_projects_helper_book_pdf_form_alter(&$form, &$form_state) {
  $book_nid = $form['nid']['#value'];
  $book_node = node_load($book_nid); //may be a book node or a book-page node
  $has_project = _erpal_projects_helper_has_project($book_node);
  if ($has_project) {
    $form['export_tasks'] = array(
      '#type' => 'checkbox',
      '#title' => t('Export tasks'),
      '#default_value' => false,
      '#weight' => -1,
    );
  }
}

/**
* Implements hook_jstree_context_menu to provide custom menu for book js tree, dependent on the node type
*/
function erpal_projects_helper_jstree_context_menu($entity_type, $type, $id, $root_id) {
  
  $current_viewed_url = $_SESSION['view_jstree']['current_tree_url']; //@TODO dirty but works, if we delete a node in the tree we want to be redirected to the curent tree view againt und this is where we are actually.
  $query_back = array('destination' => $current_viewed_url);
  
  $parent_node = node_load($id);

  $has_project = _erpal_projects_helper_has_project($parent_node);
  
  if ($type == 'erpal_task') {
    
    $menu = array(
      array('title' => t('View'), 'url' => url('node/'.$id)),
      array('title' => t('Edit'), 'url' => url('node/'.$id.'/edit', array('query' => $query_back))),
      array('title' => t('Delete'), 'url' => url('node/'.$id.'/delete', array('query' => $query_back))),            
    );
    
    if ($has_project) {
      $menu[] = array('title' => t('Add sub task'), 'url' => url('node/add/erpal-task/'.$id, array('query' => $query_back)));
      $menu[] = array('title' => t('Add timetracking'), 'url' => url('node/add/erpal-timetracking/'.$id, array('query' => $query_back)));
    }
    
    return $menu;
  }
  
  if ($type == 'erpal_book_page') {            
    if ($has_project) {
      $menu = array(
        array('title' => t('Add task'), 'url' => url('node/add/erpal-task/'.$id, array('query' => $query_back))),
      );
      
      return $menu;
    }    
  }
}

/**
* Definie which node types are allowed as children of a parent in jstree
* structure is 'parent_type' => array with possible child types
* @see views_jstree.module function theme_views_jstree
*/
function erpal_projects_helper_jstree_allowed_child_types() {
  return array(
    'erpal_task' => array('erpal_task'),
  );
}