<?

//
// class:       XMLOuput
// descr:       abstract class for writing xml
// version:     class version 0.1.beta
//
// 2002-09-??   rogo    created this
//

// devnote:     this class is made to slow down the computer performance,
//              because it's only a class to define functions for an xml output
//              class. in java this would be an interface class



class XMLOutput
{
    function openElement($name, $attribs = null)
    {
        return;
    }

    function emptyElement($name, $attribs = null)
    {
        return;
    }

    function closeElement($name)
    {
        return;
    }
    
    function content($data)
    {
        return;
    }

    function comment($comment)
    {
        return;
    }
}

?>
