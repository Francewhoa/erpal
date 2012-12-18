(function ($) {

  Drupal.behaviors.timetracking = {
    attach: function (context, settings) {
      // Timer Class
      Timer = function(timetrackingId){
        this.timerId;
        this.timetrackingId = timetrackingId;
        // time
        this.hours = 0;
        this.minutes = 0;
        this.seconds = 0;
        this.setDefaultTime();
      }
      // Timer Methods
      Timer.prototype = {
        // start timer
        start: function(){
          this.timerId = setInterval(this.updateTime.bind(this), 1000);
        },
        // stop timer
        stop: function(){
          clearTimeout(this.timerId);
        },
        // Update timer
        updateTime: function(){
          var time = [], secText, minText, timeText;
          // increment time
          this.seconds++
          if(this.seconds == 60){
            this.minutes++;
            this.seconds = 0;
          }
          if(this.minutes == 60){
            this.hours++;
            this.minutes = 0;
          }
          // digitals in format 01
          secText = this.formatDigital(this.seconds);
          minText = this.formatDigital(this.minutes);
          // collect time
          if(this.hours > 0)
            time.push(this.hours)
          time.push(minText)
          time.push(secText)
          //time = [this.hours, minText, secText];
          // format time h:mm:ss
          timeText = this.implode(':', time);
          // Print time
          var id = this.timetrackingId;
          $('.timetracking_button_' + id).find('.timetracking_duration').text(timeText);
        },
        // Convert default time in format 9.99 in h:mm:ss and set default values
        setDefaultTime: function(){
          var hoursText, minutesText, defaultText, min, secondsText, sec;
          var id = this.timetrackingId;
          defaultText = $('.timetracking_button_' + id).find('.timetracking_duration').text();
          defaultText = defaultText.replace(/\s*/, '');
          // default hours
          if(defaultText.indexOf('.') != -1){
            hoursText = defaultText.split('.').slice(0)[0];
            this.hours = hoursText;
            // default minutes
            minutesText = defaultText.replace(/\d+\./, '');;
            minutesText = minutesText.replace(/ +[a+-z]+/, '');
            if(!minutesText)
              return
            if(minutesText.indexOf('0') == -1 && minutesText < 10)
              minutesText = minutesText * 10;
            min = 60 * minutesText / 100;
            min = min + '';
            // default seconds
            this.minutes = min.split('.').slice(0)[0];
            if(min.indexOf('.') != -1){
              if(secondsText = min.split('.').slice(-1)[0]){
                sec = 60 * secondsText / 100;
                this.seconds = Math.round(sec);
              }
            }
          } else {
            hoursText = defaultText.replace(/ +[a+-z]+/, '');
            this.hours = hoursText;
          }
        },
        // Digitals in format 01
        formatDigital: function(number){
          var numberText
          if(number < 10)
            numberText = '0' + number;
          else
            numberText = number;
          return numberText
        },
        // Join array elements with a string
        implode: function( glue, pieces ){	
          return ( ( pieces instanceof Array ) ? pieces.join ( glue ) : pieces );
        }
      }
      var timers = [];
      
      // Autostart
      $('.timetracking_button').each(function(index, item){
        // get id of timer
        var lastClass = $(this).attr('class').split(' ').slice(-1);
        var id = lastClass[0].split('timetracking_button_').join(''); 
        currentState = $(this).attr("rel");
        if(currentState == 'on' && !timers[id]){
          timer = new Timer(id);
          timers[id] = timer;
          timer.start();
        }
      });
      
      // Start / Stop timer button
      $('.timetracking_button:not(.in-progress)', context).click(function () {
        // get id of timer
        var lastClass = $(this).attr('class').split(' ').slice(-1);
        var id = lastClass[0].split('timetracking_button_').join('');
        $(this).addClass('in-progress');
        // initialize timer if undefined
        if(!timers[id]){
          timer = new Timer(id);
          timers[id] = timer;
        }
        // track time
        timetrackingToggle(id, timers, $(this));
        return false;
      });
    }
  };
  
  timetrackingToggle = function(id, timers, button) {
    var timer = timers[id];
    var currentState;
    //the current state is set in link, so get it
    currentState = button.attr("rel");

    //prepare variables
    if (currentState == 'on')
    {
      var toggleURL = Drupal.settings.timetracking.on.togglepath+id+'/off/ajax';
    } else 
    if (currentState == 'off')
    {
      var toggleURL = Drupal.settings.timetracking.on.togglepath+id+'/on/ajax';
    }
    
    //now change all buttons of the class. We cannot use IDs because one button could appear multiple times at one page.
    $.get(toggleURL, function(data){	
      //prepare variables
      if (currentState == 'on')
      {
        timer.stop()
        var newState = 'off';
        var newText = Drupal.settings.timetracking.off.linktext;
        var newImage = Drupal.settings.timetracking.off.imagepath;
      } else 
      if (currentState == 'off')
      {
        timer.start()
        var newState = 'on';
        var newText = Drupal.settings.timetracking.on.linktext;
        var newImage = Drupal.settings.timetracking.on.imagepath;
      }
    
      // first stop all timetrackings
      $('.timetracking_button[rel="on"]').each(function(index, button_element){
        $(this).attr("rel", 'off');  //set off state
        var lastClass = $(this).attr('class').split(' ').slice(-1);
        var otherTimerId = lastClass[0].split('timetracking_button_').join('');
        // stop other timers
        if(otherTimerId != id && timers[otherTimerId]){
          timers[otherTimerId].stop();
        }
        // set off image
        offImage = Drupal.settings.timetracking.off.imagepath
        $(this).find('.timetracking_button_image').attr("src", offImage);
        // set off text
        offText = Drupal.settings.timetracking.off.linktext;
        $(this).find('.timetracking_text').text(offText);
      });
      
      
      $('.timetracking_button_' + id).attr("rel", newState);  //set new state
      $('.timetracking_button_' + id).find('.timetracking_button_image').attr("src", newImage)//set new image
      $('.timetracking_button_' + id).find('.timetracking_text').text(newText)//set new text
      $('.timetracking_button_' + id).removeClass('in-progress');
    });
  }
  
})(jQuery);
