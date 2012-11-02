<?php
/**
 * @file
 * Enables modules and site configuration for a erpal site installation.
 */

/**
 * Implements hook_form_FORM_ID_alter() for install_configure_form().
 *
 * Allows the profile to alter the site configuration form.
 */
function erpal_form_install_configure_form_alter(&$form, $form_state) {
  // Pre-populate the site name with the server name.
  $form['site_information']['site_name']['#default_value'] = $_SERVER['SERVER_NAME'];
}
/**
 *
 * Zusaetzliche installationsaufgaben definieren
 *
 */
function erpal_install_tasks(){
  return array(
    'company_info' => array(
      'display_name' => st('Set the name and address of your company'),
      'display' => TRUE,
      // return a Form API structured array
      'type' => 'normal',
      'run' => 'INSTALL_TASK_IF_NOT_COMPLETED', 
      ),
    
    'erpal_config' => array(
      'display_name' => st('Set Erpal settings'),
      'display' => TRUE,
      'type' => 'normal',
      'run' => 'INSTALL_TASK_IF_NOT_COMPLETED',
      ),
    );
}

/**
 * Installation task "Set the name and address of your company"
 * 
 */
function company_info(){
  $node_type = 'erpal_contact';
  $form_id = $node_type . '_node_form';
  // Node anlegen
  $node = new stdClass();
  $node->type = $node_type;
  global $user;
  $node->uid = $user->uid;
  $node->name = (isset($user->name) ? $user->name : '');  
  // Formular erzeugen
  $form = drupal_get_form($form_id, $node);
  $form['field_relation_add']['#access'] = FALSE;  
  $form['#submit'][] = 'company_info_submit';
  node_object_prepare($node);
  // Formular rendern
  return drupal_render($form);
  
  //return $form;
}
function company_info_submit($form, &$form_state) {
  $form_state['rebuild'] = TRUE;
  $form_state['redirect'] = url('admin', array('absolute' => true));

}

/**
 * 
 *
 */
function erpal_config(){
  
}


