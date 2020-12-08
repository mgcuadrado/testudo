<?xml version = "1.0" encoding = "UTF-8"?>
<xsl:stylesheet version = "1.0"
                xmlns:xsl = "http://www.w3.org/1999/XSL/Transform">

  <xsl:output method="text"/>

  <xsl:template match="interactive_test">
    <xsl:text>interactive test</xsl:text>
  </xsl:template>

  <xsl:template match="testudo">
    <xsl:apply-templates select="test"/>
  </xsl:template>

  <xsl:template match="test">
    <xsl:text>||~cartouche~||</xsl:text>
    <xsl:value-of select="@location"/>
    <xsl:text>&#xa;</xsl:text>
    <xsl:text>||~ident~||{</xsl:text>
    <xsl:choose>
      <xsl:when test="
          substring(@name,
                    (string-length(@name) - string-length(@title)) + 1)
          =@title">
        <xsl:value-of select="
            substring(@name,
                      1, (string-length(@name) - string-length(@title)))"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="@name"/>
      </xsl:otherwise>
    </xsl:choose>
    <xsl:text>}||~normal~|| </xsl:text>
    <xsl:text>||~bold~||</xsl:text>
    <xsl:value-of select="@title"/>
    <xsl:text>||~normal~||</xsl:text>
    <xsl:text>&#xa;</xsl:text>

    <xsl:apply-templates select="*"/>

    <xsl:text>||~ident~||{</xsl:text>
    <xsl:value-of select="@name"/>
    <xsl:text>}</xsl:text>
    <xsl:text>||~normal~|| </xsl:text>
      <xsl:call-template name="summary"/>
    <xsl:text>&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match="scope">
    <xsl:call-template name="brief_location"/>
    <xsl:text>||~ident~||#||~normal~|| ||~ident~||{||~normal~||</xsl:text>
    <xsl:if test="@name!=''">
      <xsl:text> ||~ident~||"||~normal~|| </xsl:text>
      <xsl:value-of select="@name"/>
      <xsl:text> ||~ident~||"||~normal~||</xsl:text>
    </xsl:if>
    <xsl:text>&#xa;</xsl:text>
    <xsl:text>||~inc_indent~||&#xa;</xsl:text>

    <xsl:apply-templates select="*"/>

    <xsl:text>||~dec_indent~||&#xa;</xsl:text>
    <xsl:text>||~ident~||#||~normal~|| ||~ident~||}||~normal~||</xsl:text>
    <xsl:if test="@name!=''">
      <xsl:text> ||~ident~||"||~normal~|| </xsl:text>
      <xsl:value-of select="@name"/>
      <xsl:text> ||~ident~||"||~normal~||</xsl:text>
    </xsl:if>
    <xsl:text>&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match="declare_scope">
    <xsl:call-template name="brief_location"/>
    <xsl:text>||~ident~||#||~normal~|| ||~ident~||{||~normal~||</xsl:text>
    <xsl:text> ||~ident~||:||~normal~|| </xsl:text>
    <xsl:value-of select="@declare"/>
    <xsl:text>&#xa;</xsl:text>
    <xsl:text>||~inc_indent~||&#xa;</xsl:text>

    <xsl:apply-templates select="*"/>

    <xsl:text>||~dec_indent~||&#xa;</xsl:text>
    <xsl:text>||~ident~||#||~normal~|| ||~ident~||}||~normal~||</xsl:text>
    <xsl:text>&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match="with">
    <xsl:call-template name="brief_location"/>
    <xsl:text>||~ident~||~||~normal~|| </xsl:text>
    <xsl:value-of select="@var"/>
    <xsl:text> in </xsl:text>
    <xsl:value-of select="@container_first"/>
    <xsl:if test="@container_rest!=''">
      <xsl:text>&#xa;</xsl:text>
      <xsl:text>||~multiline_begin~||</xsl:text>
      <xsl:value-of select="@container_rest"/>
      <xsl:text>||~multiline_end~||</xsl:text>
    </xsl:if>
    <xsl:text>&#xa;</xsl:text>
    <xsl:text>||~inc_indent~||&#xa;</xsl:text>

    <xsl:apply-templates select="*"/>

    <xsl:if test="@n_total!=''">
      <xsl:text>||~ident~||{</xsl:text>
      <xsl:value-of select="@summary"/>
      <xsl:text>}</xsl:text>
      <xsl:text>||~normal~|| </xsl:text>
      <xsl:call-template name="summary"/>
    </xsl:if>
    <xsl:text>||~dec_indent~||&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match="with_results">
    <xsl:text>||~inc_indent~||&#xa;</xsl:text>

    <xsl:apply-templates select="*"/>

    <xsl:text>||~dec_indent~||&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match="step_id">
    <xsl:text>||~ident~||[||~normal~|| </xsl:text>
    <xsl:value-of select="@id"/>
    <xsl:text> ||~ident~||]||~normal~||&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match="text">
    <xsl:call-template name="brief_location"/>
    <xsl:text>||~ident~||"||~normal~|| </xsl:text>
    <xsl:value-of select="."/>
    <xsl:text> ||~ident~||"||~normal~||&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match="multiline_text">
    <xsl:text>||~multiline_begin~||</xsl:text>
    <xsl:value-of select="."/>
    <xsl:text>||~multiline_end~||&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match="declare">
    <xsl:call-template name="brief_location"/>
    <xsl:text>||~ident~||:||~normal~|| </xsl:text>
    <xsl:value-of select="."/>
    <xsl:text> ||~ident~||;||~normal~||&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match="perform">
    <xsl:call-template name="brief_location"/>
    <xsl:text>||~ident~||#||~normal~|| </xsl:text>
    <xsl:value-of select="."/>
    <xsl:text> ||~ident~||;||~normal~||&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match="try">
    <xsl:call-template name="brief_location"/>
    <xsl:text>||~ident~||&amp;||~normal~|| </xsl:text>
    <xsl:value-of select="."/>
  </xsl:template>

  <xsl:template match="catch">
    <xsl:if test="@exception_type!=''">
      <xsl:text> ||~ident~||&gt;||~normal~|| </xsl:text>
      <xsl:value-of select="@exception_type"/>
    </xsl:if>
    <xsl:text> ||~ident~||&gt;||~normal~|| ||~ident~||"||~normal~|| </xsl:text>
    <xsl:value-of select="."/>
    <xsl:text> ||~ident~||"||~normal~||</xsl:text>
    <xsl:apply-templates select="@success"/>
    <xsl:text>&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match="check_true">
    <xsl:call-template name="brief_location"/>
    <xsl:text>||~ident~||%||~normal~|| </xsl:text>
    <xsl:apply-templates select="@prefix"/>
    <xsl:value-of select="expression1"/>
    <xsl:if test="@success='false'">
      <xsl:text> ||~ident~||:||~normal~|| false</xsl:text>
    </xsl:if>
    <xsl:apply-templates select="@success"/>
    <xsl:text>&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match="check_true_for">
    <xsl:call-template name="brief_location"/>
    <xsl:text>||~ident~||%||~normal~|| </xsl:text>
    <xsl:apply-templates select="@prefix"/>
    <xsl:value-of select="expression1"/>
    <xsl:if test="@success='false'">
      <xsl:text> ||~ident~||:||~normal~|| false</xsl:text>
    </xsl:if>
    <xsl:if test="@success='false'">
      <xsl:text> ||~ident~||?||~normal~|| </xsl:text>
      <xsl:value-of select="expressionv"/>
      <xsl:text> ||~ident~||:||~normal~|| </xsl:text>
      <xsl:value-of select="expressionv/@value"/>
    </xsl:if>
    <xsl:apply-templates select="@success"/>
    <xsl:text>&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match="check_equal">
    <xsl:call-template name="brief_location"/>
    <xsl:text>||~ident~||%||~normal~|| </xsl:text>
    <xsl:apply-templates select="@prefix"/>
    <xsl:value-of select="expression1"/>
    <xsl:text> ||~ident~||==||~normal~|| </xsl:text>
    <xsl:value-of select="expression2"/>
    <xsl:if test="@success='false'">
      <xsl:text> ||~ident~||:||~normal~|| </xsl:text>
      <xsl:value-of select="expression1/@value"/>
      <xsl:text> ||~ident~||==||~normal~|| </xsl:text>
      <xsl:value-of select="expression2/@value"/>
    </xsl:if>
    <xsl:apply-templates select="@success"/>
    <xsl:text>&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match="check_approx">
    <xsl:call-template name="brief_location"/>
    <xsl:text>||~ident~||%||~normal~|| </xsl:text>
    <xsl:apply-templates select="@prefix"/>
    <xsl:value-of select="expression1"/>
    <xsl:text> ||~ident~||//||~normal~|| </xsl:text>
    <xsl:value-of select="expression2"/>
    <xsl:text> ||~ident~||+/-||~normal~|| </xsl:text>
    <xsl:value-of select="@max_error"/>
    <xsl:if test="@success='false'">
      <xsl:text> ||~ident~||:||~normal~|| </xsl:text>
      <xsl:value-of select="expression1/@value"/>
      <xsl:text> ||~ident~||//||~normal~|| </xsl:text>
      <xsl:value-of select="expression2/@value"/>
    </xsl:if>
    <xsl:apply-templates select="@success"/>
    <xsl:text>&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match="check_verify">
    <xsl:call-template name="brief_location"/>
    <xsl:text>||~ident~||%||~normal~|| </xsl:text>
    <xsl:apply-templates select="@prefix"/>
    <xsl:value-of select="expression1"/>
    <xsl:text> ||~ident~||~||~normal~|| </xsl:text>
    <xsl:value-of select="predicate"/>
    <xsl:if test="@success='false'">
      <xsl:text> ||~ident~||:||~normal~|| </xsl:text>
      <xsl:value-of select="expression1/@value"/>
    </xsl:if>
    <xsl:apply-templates select="@success"/>
    <xsl:text>&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match="uncaught_exception">
    <xsl:text>||~ident~||&gt;||~normal~|| uncaught exception </xsl:text>
    <xsl:text>||~ident~||"||~normal~|| </xsl:text>
    <xsl:value-of select="."/>
    <xsl:text> ||~ident~||"||~normal~||</xsl:text>
    <xsl:call-template name="flag_error"/>
    <xsl:text>&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match="show_value">
    <xsl:call-template name="brief_location"/>
    <xsl:text>||~ident~||?||~normal~|| </xsl:text>
    <xsl:value-of select="expression1"/>
    <xsl:text> ||~ident~||:||~normal~|| </xsl:text>
    <xsl:value-of select="expression1/@value"/>
    <xsl:text>&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match="show_multiline_value">
    <xsl:call-template name="brief_location"/>
    <xsl:text>||~ident~||?||~normal~|| </xsl:text>
    <xsl:value-of select="expression1"/>
    <xsl:text> ||~ident~||:||~normal~||&#xa;||~multiline_begin~||</xsl:text>
    <xsl:value-of select="expression1/@value"/>
    <xsl:text>||~multiline_end~||&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match="separator">
    <xsl:call-template name="brief_location"/>
    <xsl:text>||~lines~||</xsl:text>
    <xsl:text>||~right~||-</xsl:text>
    <xsl:text>||~normal~||&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match="@success">
    <xsl:if test=".='true'">
      <xsl:text>||~right~|| [||~successtag~|| OK ||~normal~||]</xsl:text>
    </xsl:if>
    <xsl:if test=".='false'">
      <xsl:text>||~failure~|| ||~right~||- ||~normal~||</xsl:text>
      <xsl:text>[||~failuretag~||FAIL||~normal~||]</xsl:text>
    </xsl:if>
  </xsl:template>

  <xsl:template name="flag_error">
    <xsl:text>||~failure~|| ||~right~||- ||~normal~||</xsl:text>
    <xsl:text>[||~errortag~||ERR-||~normal~||]</xsl:text>
  </xsl:template>

  <xsl:template name="brief_location">
    <xsl:if test="@brief_location!=''">
      <xsl:text>||~lines~||</xsl:text>
      <xsl:value-of select="@brief_location"/>
      <xsl:text>||~normal~|| </xsl:text>
    </xsl:if>
  </xsl:template>

  <xsl:template match="@prefix">
    <xsl:if test=".!=''">
      <xsl:text>||~ident~||</xsl:text>
      <xsl:value-of select="."/>
      <xsl:text>||~normal~|| </xsl:text>
    </xsl:if>
  </xsl:template>

  <xsl:template name="summary">
    <xsl:text>||~ident~||</xsl:text>
    <xsl:value-of select="@n_failed"/>/<xsl:value-of select="@n_total"/>
    <xsl:text> fail</xsl:text>
    <xsl:if test="@n_errors!='0'">
      <xsl:text>, </xsl:text>
      <xsl:value-of select="@n_errors"/>
      <xsl:text> err</xsl:text>
    </xsl:if>
    <xsl:text>||~normal~||</xsl:text>
    <xsl:choose>
      <xsl:when test="@n_errors='0'">
        <xsl:apply-templates select="@success"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:call-template name="flag_error"/>
      </xsl:otherwise>
    </xsl:choose>
    <xsl:text>&#xa;</xsl:text>
  </xsl:template>
</xsl:stylesheet>
