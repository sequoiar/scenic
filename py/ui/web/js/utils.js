
///////////////////
/* General stuff */
///////////////////

// Extend Element.Events to add a event for the enter/return key
Element.Events.enter = {
    base: 'keyup',
    condition: function(event){
        return (event.key == 'enter');
    }
};

// Extend Element.Events to add a event for the escape key
Element.Events.escape = {
    base: 'keyup',
    condition: function(event){
        return (event.key == 'esc');
    }
};
